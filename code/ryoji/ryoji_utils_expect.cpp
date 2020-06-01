#ifndef __RYOJI_EXPECT__
#define __RYOJI_EXPECT__

#if 0
// TODO(Momo): What is Value and Error are same type...?
// - Do we expect users to wrap one of the type with another type?

// NOTE(Momo): Why we use constructors/destructors
// - There exist a valid/invalid state for this struct on creation.
// - Special case because we are dealing with union
// - There are type conversations that I want to be implicit
//
template<typename Value, typename Error>
struct Expect {
    union {
        Value value;
        Error error;
    };
    bool isError = false;
    
    // We need placement new to emulate member initialization schementics...
    // because we can't do this correctly in member initialization step...
    Expect(const Expect& rhs) : isError(rhs.isError) {
        isError ? new(&error) Error(rhs.error) : new(&value) Value(rhs.value);
    }
    Expect(Expect&& rhs) : isError(rhs.isError) {
        isError ? new(&error) Error(std::move(rhs.error)) : new(&value) Value(std::move(rhs.value));
    }
    ~Expect() {
        isError ? error.~Error() : value.~Value();
    }
    
    Expect(const Value& rhs) : value(rhs), isError(false) {}
    Expect(const Error& rhs) : error(rhs), isError(true) {}
    Expect(Value&& rhs) : value(std::move(rhs)), isError(false) {}
    Expect(Error&& rhs) : error(std::move(rhs)), isError(true) {}
    
    inline operator bool() const noexcept { return !isError; }
    
    // Note: I have thought about writing accessors to 'value' and 'error' that will panic and die
    // when, say, retrieving 'error' when 'isError' is false. But seeing that there is already a way
    // to check via 'isError', I don't feel that creating a function to check again is helpful.
    // I could leave it up to extension outside of the class. I think that makes sense :)
};


template<typename Error>
struct Expect<void, Error> {
    Error error;
    bool isError = false;
    
    Expect(const Error& rhs) : error(rhs), isError(true) {}
    Expect(Error&& rhs) : error(std::move(rhs)) {}
    Expect(const Expect& rhs) : isError(rhs.isError) {
        if (!isError) new(&error) Error(rhs.error)
    }
    Expect(Expect&& rhs) : isError(rhs.isError) {
        if (!isError) new(&error) Error(std::move(rhs.error))
    }
    ~Expect() {
        if (!isError) error.~Error();
    }
    
    inline operator bool() const noexcept{ return !isError; }
};


#endif

#endif