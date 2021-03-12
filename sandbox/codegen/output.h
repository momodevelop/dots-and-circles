

// A container for arrays, with bounds checking assert.
// The point of this, I guess, is the option to store the Count on the stack
// and the Elements on the heap.
//

struct array_int {
    usize Count;
    int* Elements;
};


static inline array_array_int
ArrayInt_Create(int* Elements, usize Count) {
    array_int Ret = {};
    Ret.Elements = Elements;
    Ret.Count = Count;

    return Ret;
}
static inline u32
ArrayInt_Find(array_int* Arr, u32 Item) {
	for (u32 I = 0; I < Arr->Count; ++I) {
		if (CompareIntUI32(Arr->E[I], Item)) {
			return I;
		}	
	}
}