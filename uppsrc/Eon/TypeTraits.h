#ifndef _Eon_TypeTraits_h_
#define _Eon_TypeTraits_h_

namespace Ecs {

template <typename Base, typename... Ts>
using AllRefBaseOf = IsAllTrue<std::is_base_of<Base, typename Ts::Type>::value...>;

template<typename T>
using IsComponent = std::is_base_of<ComponentBase, T>;

template<typename T>
using IsSystem = std::is_base_of<SystemBase, T>;

template<typename... Ts>
using AllComponents = IsAllBaseOf<ComponentBase, Ts...>;

template<typename... Ts>
using AllSystems = IsAllBaseOf<SystemBase, Ts...>;

template <typename Tuple>
struct TupleAllComponents : std::false_type {};

template <typename... Ts>
struct TupleAllComponents<Tuple<Ts...>> : AllRefBaseOf<ComponentBase, Ts...> {};

}

#endif
