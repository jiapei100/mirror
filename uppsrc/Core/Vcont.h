#ifdef _DEBUG
void BreakWhenPicked(void *ptr);
void BREAK_WHEN_PICKED__(void *ptr);

template <class T>
void BREAK_WHEN_PICKED(T& x)
{
	BREAK_WHEN_PICKED__(&x);
}
#endif

template <class T>
inline void DeepCopyConstructFill(T *t, const T *end, const T& x) {
	while(t != end)
		new(t++) T(clone(x));
}

template <class T>
inline void Construct(T *t, const T *lim) {
	while(t < lim)
		new(t++) T;
}

template <class T>
inline void Destroy(T *t, const T *end)
{
	while(t != end) {
		t->T::~T();
		t++;
	}
}

template <class T, class S>
inline void DeepCopyConstruct(T *t, const S *s, const S *end) {
	while(s != end)
		new (t++) T(clone(*s++));
}

template <class T>
class Buffer : Moveable< Buffer<T> > {
	T *ptr;

public:
	operator T*()                        { return ptr; }
	operator const T*() const            { return ptr; }
	T *operator~()                       { return ptr; }
	const T *operator~() const           { return ptr; }
	T          *Get()                    { return ptr; }
	const T    *Get() const              { return ptr; }

	void Alloc(size_t size)              { Clear(); ptr = new T[size]; }
	void Alloc(size_t size, const T& in) { Clear(); ptr = new T[size]; Fill(ptr, ptr + size, in); }

	void Clear()                         { if(ptr) delete[] ptr; ptr = NULL; }
	bool IsEmpty() const                 { return ptr == NULL; }

	Buffer()                             { ptr = NULL; }
	Buffer(size_t size)                  { ptr = new T[size]; }
	Buffer(size_t size, const T& init)   { ptr = new T[size]; Fill(ptr, ptr + size, init); }
	~Buffer()                            { if(ptr) delete[] ptr; }

	void operator=(Buffer&& v)           { if(&v != this) { Clear(); ptr = v.ptr; v.ptr = NULL; } }
	Buffer(Buffer&& v)                   { ptr = v.ptr; v.ptr = NULL; }

	Buffer(size_t size, std::initializer_list<T> init) : Buffer(size) {
		T *t = ptr; for(const auto& i : init) new (t++) T(i);
	}
	Buffer(std::initializer_list<T> init) : Buffer(init.size(), init) {}
};

template <class T>
class Vector : public MoveableAndDeepCopyOption< Vector<T> > {
	T       *vector;
	int      items;
	int      alloc;

	static void    RawFree(T *ptr)            { if(ptr) MemoryFree(ptr); }
	static T      *RawAlloc(int& n);

	void     Zero()                          { vector = NULL; items = alloc = 0; }
	void     Pick(Vector<T>&& v);

	T       *Rdd()                           { return vector + items++; }

	void     Free();
	void     __DeepCopy(const Vector& src);
	T&       Get(int i) const        { ASSERT(i >= 0 && i < items); return vector[i]; }
	void     ReAlloc(int alloc);
	void     ReAllocF(int alloc);
	void     Grow();
	void     GrowF();
	T&       GrowAdd(const T& x);
	T&       GrowAddPick(T&& x);
	void     RawInsert(int q, int count);

public:
	T&       Add()                   { if(items >= alloc) GrowF(); return *(::new(Rdd()) T); }
	T&       Add(const T& x)         { return items < alloc ? *(new(Rdd()) T(clone(x))) : GrowAdd(x); }
	T&       Add(T&& x)              { return items < alloc ? *(::new(Rdd()) T(pick(x))) : GrowAddPick(pick(x)); }
	template <class... Args>
	T&       Create(Args... args)    { if(items >= alloc) GrowF(); return *(::new(Rdd()) T(args...)); }
	void     AddN(int n);
	const T& operator[](int i) const { return Get(i); }
	T&       operator[](int i)       { return Get(i); }
	int      GetCount() const        { return items; }
	bool     IsEmpty() const         { return items == 0; }
	void     Trim(int n);
	void     SetCount(int n);
	void     SetCount(int n, const T& init);
	void     SetCountR(int n);
	void     SetCountR(int n, const T& init);
	void     Clear();

	T&       At(int i)                  { if(i >= items) SetCountR(i + 1); return (*this)[i]; }
	T&       At(int i, const T& x)      { if(i >= items) SetCountR(i + 1, x); return (*this)[i]; }

	void     Shrink()                   { if(items != alloc) ReAllocF(items); }
	void     Reserve(int n);
	int      GetAlloc() const           { return alloc; }

	void     Set(int i, const T& x, int count);
	T&       Set(int i, const T& x)     { Set(i, x, 1); return Get(i); }
	template <class Range>
	void     SetRange(int i, const Range& r);

	void     Remove(int i, int count = 1);
	void     Remove(const int *sorted_list, int n);
	void     Remove(const Vector<int>& sorted_list);

	void     InsertN(int i, int count = 1);
	T&       Insert(int i)              { InsertN(i); return Get(i); }
	void     Insert(int i, const T& x, int count);
	T&       Insert(int i, const T& x)  { Insert(i, x, 1); return Get(i); }
	T&       Insert(int i, T&& x);
	void     Insert(int i, const Vector& x);
	void     Insert(int i, const Vector& x, int offset, int count);
	void     Insert(int i, Vector&& x);
	template <class Range>
	void     InsertRange(int i, const Range& r);
	void     Append(const Vector& x)               { Insert(GetCount(), x); }
	void     Append(const Vector& x, int o, int c) { Insert(GetCount(), x, o, c); }
	void     Append(Vector&& x)                    { Insert(GetCount(), pick(x)); }
	template <class Range>
	void     AppendRange(const Range& r)           { InsertRange(GetCount(), r); }

	void     InsertSplit(int i, Vector<T>& v, int from);

	void     Swap(int i1, int i2)    { UPP::Swap(Get(i1), Get(i2)); }

	void     Drop(int n = 1)         { ASSERT(n <= GetCount()); Trim(items - n); }
	T&       Top()                   { ASSERT(GetCount()); return Get(items - 1); }
	const T& Top() const             { ASSERT(GetCount()); return Get(items - 1); }
	T        Pop()                   { T h = pick(Top()); Drop(); return h; }

	operator T*()                    { return (T*)vector; }
	operator const T*() const        { return (T*)vector; }

	Vector&  operator<<(const T& x)  { Add(x); return *this; }

#ifdef UPP
	void     Serialize(Stream& s)    { StreamContainer(s, *this); }
	void     Xmlize(XmlIO& xio, const char *itemtag = "item");
	void     Jsonize(JsonIO& jio);
	String   ToString() const;
	dword    GetHashValue() const    { return HashBySerialize(*this); }
	template <class B> bool operator==(const B& b) const { return IsEqualRange(*this, b); }
	template <class B> bool operator!=(const B& b) const { return !operator==(b); }
	template <class B> int  Compare(const B& b) const    { return CompareRanges(*this, b); }
	template <class B> bool operator<=(const B& x) const { return Compare(x) <= 0; }
	template <class B> bool operator>=(const B& x) const { return Compare(x) >= 0; }
	template <class B> bool operator<(const B& x) const  { return Compare(x) < 0; }
	template <class B> bool operator>(const B& x) const  { return Compare(x) > 0; }
#endif

	Vector()                         { Zero(); }
	~Vector() {
		Free();
		return; // Constraints:
		AssertMoveable((T *)0);  // T must be moveable
	}

// Pick assignment & copy. Picked source can only do Clear(), ~Vector(), operator=, operator <<=
	Vector(Vector&& v)               { Pick(pick(v)); }
	void operator=(Vector&& v)       { if(this != &v) { Free(); Pick(pick(v)); } }

	void     Insert(int i, std::initializer_list<T> init);
	void     Append(std::initializer_list<T> init) { Insert(GetCount(), init); }
	Vector(std::initializer_list<T> init) { vector = NULL; items = alloc = 0; Insert(0, init); }

// Deep copy
	Vector(const Vector& v, int)     { __DeepCopy(v); }

// Standard container interface
	const T         *begin() const          { return (T*)vector; }
	const T         *end() const            { return (T*)vector + items; }
	T               *begin()                { return (T*)vector; }
	T               *end()                  { return (T*)vector + items; }

// Optimizations
	friend void Swap(Vector& a, Vector& b)  { UPP::Swap(a.items, b.items); UPP::Swap(a.alloc, b.alloc); UPP::Swap(a.vector, b.vector); }

#ifdef DEPRECATED
	T&       DoIndex(int i)             { return At(i); }
	T&       DoIndex(int i, const T& x) { return At(i, x); }
	T&       AddPick(T&& x)             { return items < alloc ? *(::new(Rdd()) T(pick(x))) : GrowAddPick(pick(x)); }
	int      GetIndex(const T& item) const;
	T&       InsertPick(int i, T&& x)   { Insert(i, pick(x)); }
	void     InsertPick(int i, Vector&& x) { Insert(i, pick(x)); }
	void     AppendPick(Vector&& x)                { InsertPick(GetCount(), pick(x)); }
	typedef T       *Iterator;
	typedef const T *ConstIterator;
	ConstIterator    GetIter(int i) const   { ASSERT(i >= 0 && i <= items); return begin() + i; }
	Iterator         GetIter(int i)         { ASSERT(i >= 0 && i <= items); return begin() + i; }
	typedef T        ValueType;
	friend void Append(Vector& dst, const Vector& src)         { dst.Append(src); }

	STL_VECTOR_COMPATIBILITY(Vector<T>)
#endif
};

template <class T>
class Array : public MoveableAndDeepCopyOption< Array<T> > {
protected:
#ifdef _DEBUG
	typedef T *PointerType;
#else
	typedef void *PointerType;
#endif
	Vector<PointerType> vector;

	void     Free();
	void     __DeepCopy(const Array& v);
	T&       Get(int i) const                           { return *(T *)vector[i]; }
	T      **GetPtr(int i) const                        { return (T **)vector.begin() + i; }

	void     Del(PointerType *ptr, PointerType *lim)              { while(ptr < lim) delete (T *) *ptr++; }
	void     Init(PointerType *ptr, PointerType *lim)             { while(ptr < lim) *ptr++ = new T; }
	void     Init(PointerType *ptr, PointerType *lim, const T& x) { while(ptr < lim) *ptr++ = new T(clone(x)); }

public:
	T&       Add()                      { T *q = new T; vector.Add(q); return *q; }
	T&       Add(const T& x)            { T *q = new T(clone(x)); vector.Add(q); return *q; }
	T&       Add(T&& x)                 { T *q = new T(pick(x)); vector.Add(q); return *q; }
	T&       Add(T *newt)               { vector.Add(newt); return *newt; }
	template<class TT, class... Args>
	TT& Create(Args... args)            { TT *q = new TT(args...); Add(q); return *q; }
	const T& operator[](int i) const    { return Get(i); }
	T&       operator[](int i)          { return Get(i); }
	int      GetCount() const           { return vector.GetCount(); }
	bool     IsEmpty() const            { return vector.IsEmpty(); }
	void     Trim(int n);
	void     SetCount(int n);
	void     SetCount(int n, const T& init);
	void     SetCountR(int n);
	void     SetCountR(int n, const T& init);
	void     Clear()                    { Free(); vector.Clear(); }

	T&       At(int i)                  { if(i >= GetCount()) SetCountR(i + 1); return Get(i); }
	T&       At(int i, const T& x)      { if(i >= GetCount()) SetCountR(i + 1, x); return Get(i); }

	void     Shrink()                   { vector.Shrink(); }
	void     Reserve(int xtra)          { vector.Reserve(xtra); }
	int      GetAlloc() const           { return vector.GetAlloc(); }

	void     Set(int i, const T& x, int count);
	T&       Set(int i, const T& x)     { Set(i, x, 1); return Get(i); }
	void     Remove(int i, int count = 1);
	void     Remove(const int *sorted_list, int n);
	void     Remove(const Vector<int>& sorted_list);
	void     InsertN(int i, int count = 1);
	T&       Insert(int i)              { InsertN(i); return Get(i); }
	void     Insert(int i, const T& x, int count);
	T&       Insert(int i, const T& x)  { Insert(i, x, 1); return Get(i); }
	T&       Insert(int i, T&& x);
	void     Insert(int i, const Array& x);
	void     Insert(int i, const Array& x, int offset, int count);
	template <class Range>
	void     InsertRange(int i, const Range& r);
	void     Insert(int i, Array&& x)             { vector.InsertPick(i, pick(x.vector)); }
	void     Append(const Array& x)               { Insert(GetCount(), x); }
	void     Append(const Array& x, int o, int c) { Insert(GetCount(), x, o, c); }
	void     Append(Array&& x)                    { InsertPick(GetCount(), pick(x)); }
	template <class Range>
	void     AppendRange(const Range& r)          { InsertRange(GetCount(), r); }

	void     Swap(int i1, int i2)                 { UPP::Swap(vector[i1], vector[i2]); }
	void     Move(int i1, int i2);

	T&       Set(int i, T *newt)        { delete (T *)vector[i]; vector[i] = newt; return *newt; }
	T&       Insert(int i, T *newt);

	void     Drop(int n = 1)            { Trim(GetCount() - n); }
	T&       Top()                      { return Get(GetCount() - 1); }
	const T& Top() const                { return Get(GetCount() - 1); }

	T       *Detach(int i)              { T *t = &Get(i); vector.Remove(i); return t; }
	T       *Swap(int i, T *newt)       { T *tmp = (T*)vector[i]; vector[i] = newt; return tmp; }
	T       *PopDetach()                { return (T *) vector.Pop(); }

	void     Swap(Array& b)             { Swap(vector, b.vector); }

	Array& operator<<(const T& x)       { Add(x); return *this; }
	Array& operator<<(T *newt)          { Add(newt); return *this; }

	void     Serialize(Stream& s)       { StreamContainer(s, *this); }
	void     Xmlize(XmlIO& xio, const char *itemtag = "item");
	void     Jsonize(JsonIO& jio);
	String   ToString() const;
	dword    GetHashValue() const    { return HashBySerialize(*this); }

	template <class B> bool operator==(const B& b) const { return IsEqualRange(*this, b); }
	template <class B> bool operator!=(const B& b) const { return !operator==(b); }
	template <class B> int  Compare(const B& b) const    { return CompareRanges(*this, b); }
	template <class B> bool operator<=(const B& x) const { return Compare(x) <= 0; }
	template <class B> bool operator>=(const B& x) const { return Compare(x) >= 0; }
	template <class B> bool operator<(const B& x) const  { return Compare(x) < 0; }
	template <class B> bool operator>(const B& x) const  { return Compare(x) > 0; }

	Array()                             {}
	~Array()                            { Free(); }

// Pick assignment & copy. Picked source can only Clear(), ~Vector(), operator=, operator<<=
	Array(Array&& v) : vector(pick(v.vector))  {}
	void operator=(Array&& v)           { if(this != &v) { Free(); vector = pick(v.vector); } }

// Deep copy
	Array(const Array& v, int)          { __DeepCopy(v); }

	void     Insert(int i, std::initializer_list<T> init);
	void     Append(std::initializer_list<T> init) { Insert(GetCount(), init); }
	Array(std::initializer_list<T> init) { Insert(0, init); }

	class Iterator;

	class ConstIterator {
	protected:
		T **ptr;
		ConstIterator(T **p)                    { ptr = p; }

		friend class Array<T>;
		struct NP { int dummy; };

	public:
		const T& operator*() const              { return **ptr; }
		const T *operator->() const             { return *ptr; }
		const T& operator[](int i) const        { return *ptr[i]; }

		ConstIterator& operator++()             { ptr++; return *this; }
		ConstIterator& operator--()             { ptr--; return *this; }
		ConstIterator  operator++(int)          { ConstIterator t = *this; ++*this; return t; }
		ConstIterator  operator--(int)          { ConstIterator t = *this; --*this; return t; }

		ConstIterator& operator+=(int i)        { ptr += i; return *this; }
		ConstIterator& operator-=(int i)        { ptr -= i; return *this; }

		ConstIterator operator+(int i) const    { return ptr + i; }
		ConstIterator operator-(int i) const    { return ptr - i; }

		int  operator-(ConstIterator x) const   { return (int)(ptr - x.ptr); }

		bool operator==(ConstIterator x) const  { return ptr == x.ptr; }
		bool operator!=(ConstIterator x) const  { return ptr != x.ptr; }
		bool operator<(ConstIterator x) const   { return ptr < x.ptr; }
		bool operator>(ConstIterator x) const   { return ptr > x.ptr; }
		bool operator<=(ConstIterator x) const  { return ptr <= x.ptr; }
		bool operator>=(ConstIterator x) const  { return ptr >= x.ptr; }

		operator bool() const                   { return ptr; }

		ConstIterator()                         {}
		ConstIterator(NP *null)                 { ASSERT(null == NULL); ptr = NULL; }
		
		STL_ITERATOR_COMPATIBILITY
	};

	class Iterator : public ConstIterator {
		friend class Array<T>;
		Iterator(T **p) : ConstIterator(p)      {}
		typedef ConstIterator B;
		struct NP { int dummy; };

	public:
		T& operator*()                          { return **B::ptr; }
		T *operator->()                         { return *B::ptr; }
		T& operator[](int i)                    { return *B::ptr[i]; }

		const T& operator*() const              { return **B::ptr; }
		const T *operator->() const             { return *B::ptr; }
		const T& operator[](int i) const        { return *B::ptr[i]; }

		Iterator& operator++()                  { B::ptr++; return *this; }
		Iterator& operator--()                  { B::ptr--; return *this; }
		Iterator  operator++(int)               { Iterator t = *this; ++*this; return t; }
		Iterator  operator--(int)               { Iterator t = *this; --*this; return t; }

		Iterator& operator+=(int i)             { B::ptr += i; return *this; }
		Iterator& operator-=(int i)             { B::ptr -= i; return *this; }

		Iterator operator+(int i) const         { return B::ptr + i; }
		Iterator operator-(int i) const         { return B::ptr - i; }

		int      operator-(Iterator x) const    { return B::operator-(x); }

		Iterator()                               {}
		Iterator(NP *null) : ConstIterator(null) {}

		STL_ITERATOR_COMPATIBILITY
	};

// Standard container interface
	Iterator         begin()                    { return (T **)vector.Begin(); }
	Iterator         end()                      { return (T **)vector.End(); }
	ConstIterator    begin() const              { return (T **)vector.Begin(); }
	ConstIterator    end() const                { return (T **)vector.End(); }

// Optimalization
	friend void Swap(Array& a, Array& b)                   { UPP::Swap(a.vector, b.vector); }
	//GCC forced move from Iterator, ugly workaround
private:
	static void IterSwap0(Iterator a, Iterator b)          { UPP::Swap(*a.ptr, *b.ptr); }
public:
	friend void IterSwap(Iterator a, Iterator b)           { Array<T>::IterSwap0(a, b); }

#ifdef DEPRECATED
	int      GetIndex(const T& item) const;
	void     InsertPick(int i, Array&& x)       { vector.InsertPick(i, pick(x.vector)); }
	void     AppendPick(Array&& x)              { InsertPick(GetCount(), pick(x)); }
	T&       DoIndex(int i)                     { return At(i); }
	T&       DoIndex(int i, const T& x)         { return At(i, x); }
	T&       AddPick(T&& x)                     { T *q = new T(pick(x)); vector.Add(q); return *q; }
	T&       InsertPick(int i, T&& x)           { return Insert(i, pick(x)); }
	typedef T        ValueType;
	Iterator         GetIter(int pos)           { return (T **)vector.GetIter(pos); }
	ConstIterator    GetIter(int pos) const     { return (T **)vector.GetIter(pos); }
#endif
	STL_VECTOR_COMPATIBILITY(Array<T>)
};
