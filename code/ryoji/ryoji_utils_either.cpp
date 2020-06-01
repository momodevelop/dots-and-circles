#ifndef __RYOJI_EITHER__
#define __RYOJI_EITHER__

#if 0

template<typename Left, typename Right>
struct Either {
    union {
        Left left;
        Right right;
    };
    bool isRight;
};

template<typename Right>
struct Either<void, Right> {
    Right right;
    bool isRight = false;
};

template<typename L, typename R>
pure Either<L, R> makeEither(const L& lhs) {
    return { {.left = lhs }, false };
}

template<typename L, typename R>
inline Either<L, R> makeEither(const R& rhs) {
    return { {.right = rhs }, true };
}

template<typename L, typename R>
pure bool isEitherLeft(const Either<L, R>& lhs) {
    return !lhs.isRight;
}

template<typename L, typename R>
pure bool isEitherRight(const Either<L, R>& lhs) {
    return lhs.isRight;
}

template<typename L, typename R>
pure L& getEitherLeft(Either<L, R>& lhs) {
    assert(!lhs.isRight);
    return lhs.left;
}

template<typename L, typename R>
pure R& getEitherRight(Either<L, R>& lhs) {
    assert(lhs.isRight);
    return lhs.right;
}

#endif


#endif