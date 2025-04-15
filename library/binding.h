#pragma once

#include "utils.h"
#include <cstdio>
#include <functional>
#include <tuple>

namespace PropertyModel {

namespace Library {
namespace Details {
template<class List, size_t Index>
struct GetH;

template<class T, class... Ts>
struct GetH<std::tuple<T, Ts...>, 0> {
	using type = T;
};

template<class T, class... Ts, size_t ind>
struct GetH<std::tuple<T, Ts...>, ind> {
	using type = typename GetH<std::tuple<Ts...>, ind - 1>::type;
};

template<typename Tuple, size_t ind>
using Get = typename GetH<Tuple, ind>::type;

template<class Index, typename... TupleTupes>
struct TypeH;

template<size_t Index, typename Tuple1, typename Tuple2, typename Tuple3>
struct TypeH<Data<Index>, Tuple1, Tuple2, Tuple3> {
	using type = Get<Tuple1, Index>;
};

template<size_t Index, typename Tuple1, typename Tuple2, typename Tuple3>
struct TypeH<Value<Index>, Tuple1, Tuple2, Tuple3> {
	using type = Get<Tuple2, Index>;
};

template<size_t Index, typename Tuple1, typename Tuple2, typename Tuple3>
struct TypeH<Output<Index>, Tuple1, Tuple2, Tuple3> {
	using type = Get<Tuple3, Index>;
};
} // namespace Details

template<typename R, typename Tuple1, typename Tuple2, typename Tuple3>
using Type = typename Details::TypeH<R, Tuple1, Tuple2, Tuple3>::type;

namespace Details {

template<typename... A>
struct FunctionH;

template<typename T, typename... Ts, typename Tuple1, typename Tuple2,
		 typename Tuple3>
struct FunctionH<Tuple1, Tuple2, Tuple3, T, Ts...> {
	using type = std::function<Type<T, Tuple1, Tuple2, Tuple3>(
		Type<Ts, Tuple1, Tuple2, Tuple3>...)>;
};

} // namespace Details

template<typename Tuple1, typename Tuple2, typename Tuple3, typename... Ts>
using Function =
	typename Details::FunctionH<Tuple1, Tuple2, Tuple3, Ts...>::type;

template<class A>
struct Getter;

template<size_t Ind>
struct Getter<Data<Ind>> {
	template<typename... DataTypes, typename... ValueTypes,
			 typename... OutputTypes>
	static auto& get(std::tuple<DataTypes...>& data,
					 std::tuple<ValueTypes...>& value,
					 std::tuple<OutputTypes...>& output) {
		return std::get<Ind>(data);
	}
};

template<size_t Ind>
struct Getter<Value<Ind>> {
	template<typename... DataTypes, typename... ValueTypes,
			 typename... OutputTypes>
	static auto& get(std::tuple<DataTypes...>& data,
					 std::tuple<ValueTypes...>& value,
					 std::tuple<OutputTypes...>& output) {
		return std::get<Ind>(value);
	}
};

template<size_t Ind>
struct Getter<Output<Ind>> {
	template<typename... DataTypes, typename... ValueTypes,
			 typename... OutputTypes>
	static auto& get(std::tuple<DataTypes...>& data,
					 std::tuple<ValueTypes...>& value,
					 std::tuple<OutputTypes...>& output) {
		return std::get<Ind>(output);
	}
};

}; // namespace Library
}; // namespace PropertyModel
