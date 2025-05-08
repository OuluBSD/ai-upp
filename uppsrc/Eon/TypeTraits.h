#ifndef _Eon_TypeTraits_h_
#define _Eon_TypeTraits_h_

namespace Ecs {

class Engine;
class ComponentBase;
class SystemBase;

template <typename Base, typename... Ts>
using AllPtrBaseOf = IsAllTrue<std::is_base_of<Base, typename Ts::Type>::value...>;

template<typename T>
using IsComponent = std::is_base_of<Ecs::ComponentBase, T>;

template<typename T>
using IsSystem = std::is_base_of<Ecs::SystemBase, T>;

template<typename... Ts>
using AllComponents = IsAllBaseOf<Ecs::ComponentBase, Ts...>;

template<typename... Ts>
using AllSystems = IsAllBaseOf<Ecs::SystemBase, Ts...>;

template <typename Tuple>
struct TupleAllComponents : std::false_type {};

template <typename... Ts>
struct TupleAllComponents<Tuple<Ts...>> : AllPtrBaseOf<Ecs::ComponentBase, Ts...> {};

template <typename Tuple>
struct RTupleAllComponents : std::false_type {};

template <typename... Ts>
struct RTupleAllComponents<RTuple<Ts...>> : AllPtrBaseOf<Ecs::ComponentBase, Ts...> {};

}

#endif
