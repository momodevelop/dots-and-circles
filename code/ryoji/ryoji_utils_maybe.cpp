#ifndef __RYOJI_MAYBE__
#define __RYOJI_MAYBE__

// TODO(Momo): Maybe not :')

#if 0

template<typename T>
struct Maybe {
    T item;
    bool isNothing = true;
};

template<typename T>
inline Maybe<T> make(const T& item) {
    return { item, false };
}

template<typename T>
inline bool isYes(const Maybe<T>& lhs) {
    return !lhs.isNothing;
}

template<typename T>
inline bool isNo(const Maybe<T>& lhs) {
    return lhs.isNothing;
}

template<typename T>
inline T& get(Maybe<T>& lhs) {
    Assert(!lhs.isNothing);
    return lhs.item;
}

#endif
#endif