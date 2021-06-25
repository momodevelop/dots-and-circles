void
Big_Int::set_zero() {
    for (u32 i = 0; i < count; ++i) {
        data[i] = 0;
    }
    places = 1;
}

b8
Big_Int::init(u8* buffer, u32 length) {
    if (!buffer || length == 0) {
        return false;
    }
    data = buffer;
    count = length;
    
    set_zero();
    return true;
}


void
Big_Int::set_max() {
    for(u32 i = 0; i < count; ++i) {
        data[i] = 9;
    }
    places = count;
}

b8
Big_Int::alloc(Arena* arena, u32 length) {
    u8* buffer = arena->push_array<u8>(length);
    return init(buffer, length);
}

void
Big_Int::add(u32 value) {
    // NOTE(Momo): For each place, add
    u32 index = 0;
    u8 carry = 0;
    while (value > 0) {
        if (index >= count) {
            set_max();
            return;
        }
        u8 extracted_value = (u8)(value % 10);
        u8 result = extracted_value + carry + data[index];
        if (result >= 10) {
            carry = 1;
            result -= 10;
        }
        else {
            carry = 0;
        }
        data[index] = result; 
        value /= 10;
        ++index;
        
    }
    
    while(carry > 0) {
        if (index >= count) {
            set_max();
            return;
        }
        u8 result = data[index] + carry;
        if (result >= 10) {
            carry = 1;
            result -= 10;
        }
        else {
            carry = 0;
        }
        data[index] = result;
        ++index;
    }
    
    if (index > places) {
        places = index;
    }
}

// TODO: We can probably optimize with by not calling set_zero()
void
Big_Int::set(u32 value) {
    // NOTE(Momo): For each place, add
    u32 index = 0;
    u8 carry = 0;
    while (value > 0) {
        if (index >= count) {
            set_max();
            return;
        }
        
        data[index] = (u8)(value % 10); 
        value /= 10;
        ++index;
    }
    
    if (index > places) {
        places = index;
    }
}

Big_Int&
Big_Int::operator+=(u32 rhs) {
    add(rhs);
    return (*this);
}

Big_Int&
Big_Int::operator=(u32 rhs) {
    set(rhs);
    return (*this);
}



//~NOTE(Momo): Forward Iterator
Big_Int_Forward_Itr
Big_Int::begin() {
    return { this, 0 };
}

Big_Int_Forward_Itr
Big_Int::end() {
    return { this, places };
}

b8
Big_Int_Forward_Itr::operator!=(Big_Int_Forward_Itr rhs) {
    return index != rhs.index;
}

Big_Int_Forward_Itr&
Big_Int_Forward_Itr::operator++() {
    ++index;
    return (*this);
}

u8&
Big_Int_Forward_Itr::operator*() {
    return big_int->data[index];
}

//~NOTE(Momo): Reverse Iterator
Big_Int_Reverse_Itr
Big_Int::rbegin() {
    return { this, 0 };
}

Big_Int_Reverse_Itr
Big_Int::rend() {
    return { this, this->places };
}


b8
Big_Int_Reverse_Itr::operator!=(Big_Int_Reverse_Itr rhs) {
    return index != rhs.index;
}

Big_Int_Reverse_Itr&
Big_Int_Reverse_Itr::operator++() {
    ++index;
    return (*this);
}

u8&
Big_Int_Reverse_Itr::operator*() {
    u32 actual_index = big_int->places - index - 1;
    return big_int->data[actual_index];
}
