#ifndef _Core2_TypeTraits_h_
#define _Core2_TypeTraits_h_


template<typename T1, typename T2>
T1& AsRef(T2& val)
{
    static_assert(sizeof(T1) == sizeof(T2), "Sizes should be the same");
    return *reinterpret_cast<T1*>(&val);
}

template<typename T1, typename T2>
const T1& AsRef(const T2& val)
{
    static_assert(sizeof(T1) == sizeof(T2), "Sizes should be the same");
    return *reinterpret_cast<const T1*>(&val);
}

template<typename Container, typename Predicate>
void EraseIf(Container* container, Predicate&& predicate)
{
	for(int i = 0; i < container->GetCount(); i++) {
		if (predicate((*container)[i]))
			container->Remove(i--);
	}
    //container->Remove(RemoveIf(container->begin(), container->end(), Pick(predicate)), container->end());
}

class Destroyable
{
public:
	virtual ~Destroyable() = default;

    virtual void Destroy() { destroyed = true; }
    virtual bool IsDestroyed() const { return destroyed; }

    template<typename Container>
    static void PruneFromContainer(Container& container)
    {
        auto it = container.begin();
        while(it) {
            if (it().IsDestroyed())
                it = container.Remove(it);
            else
                ++it;
        }
    }

protected:
    bool destroyed = false;
};


class Enableable
{
public:
	virtual ~Enableable() = default;

    virtual void SetEnabled(bool enable) { m_enabled = enable; }
    virtual bool IsEnabled() const { return m_enabled; }

protected:
    bool m_enabled{ true };
};

#if 0
template<typename T, typename ProducerT, typename RefurbisherT>
class FactoryT
{
public:
    using Type = T;
    using Producer = ProducerT;
    using Refurbisher = RefurbisherT;

    void RegisterProducer(const TypeCls& typeId, Producer producer, Refurbisher refurbisher)
    {
        auto p = producers.find(typeId);
        AssertFalse(p != producers.end(), "multiple registrations for the same type is not allowed");
        producers.insert(p, { typeId, pick<Producer>(producer) });
        
        auto r = refurbishers.find(typeId);
        AssertFalse(r != refurbishers.end(), "multiple registrations for the same type is not allowed");
        refurbishers.insert(r, { typeId, pick<Refurbisher>(refurbisher) });
    }

protected:
    TypeMap<ProducerT> producers;
    TypeMap<RefurbisherT> refurbishers;
    
};
#endif

template<bool...>
struct bool_pack;

template <typename Base, typename T>
using IsBaseOf = std::is_base_of<Base, T>;

template<bool... Bs>
using IsAllTrue = std::is_same<bool_pack<Bs..., true>, bool_pack<true, Bs...>>;


template <typename Base, typename... Ts>
using IsAllBaseOf = IsAllTrue<std::is_base_of<Base, Ts>::value...>;

template <typename To, typename... Ts>
using IsAllConvertibleTo = IsAllTrue<std::is_convertible<Ts, To>::value...>;

template <typename T, typename... Ts>
using IsAllSame = IsAllTrue<std::is_same<T, Ts>::value...>;

template <typename T, typename Base>
bool IsInstance(const Base& o) {return CastConstPtr<T>(&o) != 0;}


#endif
