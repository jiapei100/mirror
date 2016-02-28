template <class I>
class SubRangeClass {
	I   l;
	int count;

public:
	typedef typename std::remove_reference<decltype(*l)>::type value_type;

	int GetCount() const { return count; }
	
	SubRangeClass& Write()                   { return *this; }

	value_type& operator[](int i) const { ASSERT(i >= 0 && i < count); return l[i]; }
	I  begin() const { return l; }
	I  end() const { return l + count; }

	String   ToString() const                            { return AsStringArray(*this); }
	template <class B> bool operator==(const B& b) const { return IsEqualArray(*this, b); }
	template <class B> bool operator!=(const B& b) const { return !operator==(b); }
	template <class B> int  Compare(const B& b) const    { return CompareArray(*this, b); }
	template <class B> bool operator<=(const B& x) const { return Compare(x) <= 0; }
	template <class B> bool operator>=(const B& x) const { return Compare(x) >= 0; }
	template <class B> bool operator<(const B& x) const  { return Compare(x) < 0; }
	template <class B> bool operator>(const B& x) const  { return Compare(x) > 0; }

	SubRangeClass(I begin, int count) : l(begin), count(count) {}
	SubRangeClass(I begin, I end)                              { l = begin; count = end - begin; }
};

template <class I>
SubRangeClass<I> SubRange(I l, I h)
{
	return SubRangeClass<I>(l, h);
}

template <class I>
SubRangeClass<I> SubRange(I l, int count)
{
	return SubRangeClass<I>(l, count);
}

template <class C>
auto SubRange(C& c, int pos, int count) -> decltype(Range(c.begin() + pos, count))
{
	return Range(c.begin() + pos, count);
}

template <class T>
struct ConstRangeClass {
	T   value;
	int count;
	
	typedef T value_type;
	typedef value_type ValueType;
	
	const value_type& operator[](int i) const { return value; }
	int GetCount() const                      { return count; }
	
	typedef ConstIIterator<ConstRangeClass> Iterator;
	
	Iterator begin() const { return Iterator(*this, 0); }
	Iterator end() const { return Iterator(*this, count); }

	String   ToString() const                            { return AsStringArray(*this); }
	template <class B> bool operator==(const B& b) const { return IsEqualArray(*this, b); }
	template <class B> bool operator!=(const B& b) const { return !operator==(b); }
	template <class B> int  Compare(const B& b) const    { return CompareArray(*this, b); }
	template <class B> bool operator<=(const B& x) const { return Compare(x) <= 0; }
	template <class B> bool operator>=(const B& x) const { return Compare(x) >= 0; }
	template <class B> bool operator<(const B& x) const  { return Compare(x) < 0; }
	template <class B> bool operator>(const B& x) const  { return Compare(x) > 0; }

	ConstRangeClass(const T& value, int count) : value(value), count(count) {}
	ConstRangeClass(int count) : count(count) {}
};

template <class T>
ConstRangeClass<T> ConstRange(const T& value, int count)
{
	return ConstRangeClass<T>(value, count);
}

template <class T>
ConstRangeClass<T> ConstRange(int count)
{
	return ConstRangeClass<T>(count);
}

template <class BaseRange>
struct FilterRangeRef {
	BaseRange  *r;
	Vector<int> ndx;
	
	typedef typename BaseRange::value_type value_type;
	typedef value_type ValueType;
	
	const value_type& operator[](int i) const { return (*r)[ndx[i]]; }
	value_type& operator[](int i)             { return (*r)[ndx[i]]; }
	int GetCount() const                      { return ndx.GetCount(); }
	
	typedef IIterator<FilterRangeRef> Iterator;
	typedef ConstIIterator<FilterRangeRef> ConstIterator;

	FilterRangeRef& Write()                   { return *this; }

	ConstIterator begin() const { return ConstIterator(*this, 0); }
	ConstIterator end() const  { return ConstIterator(*this, ndx.GetCount()); }
	
	Iterator begin() { return Iterator(*this, 0); }
	Iterator end()   { return Iterator(*this, ndx.GetCount()); }

	String   ToString() const                            { return AsStringArray(*this); }
	template <class B> bool operator==(const B& b) const { return IsEqualArray(*this, b); }
	template <class B> bool operator!=(const B& b) const { return !operator==(b); }
	template <class B> int  Compare(const B& b) const    { return CompareArray(*this, b); }
	template <class B> bool operator<=(const B& x) const { return Compare(x) <= 0; }
	template <class B> bool operator>=(const B& x) const { return Compare(x) >= 0; }
	template <class B> bool operator<(const B& x) const  { return Compare(x) < 0; }
	template <class B> bool operator>(const B& x) const  { return Compare(x) > 0; }

	FilterRangeRef(BaseRange& r, Vector<int>&& ndx) : r(&r), ndx(pick(ndx)) {}
};

template <class BaseRange>
FilterRangeRef<BaseRange> FilterRange(BaseRange& r, Vector<int>&& ndx)
{
	return FilterRangeRef<BaseRange>(r, pick(ndx));
}

template <class Range, class Predicate>
Vector<int> FindAll(const Range& r, Predicate p)
{
	Vector<int> ndx;
	for(int i = 0; i < r.GetCount(); i++)
		if(p(r[i]))
			ndx.Add(i);
	return ndx;
}

template <class BaseRange, class Predicate>
FilterRangeRef<BaseRange> FilterRange(BaseRange& r, Predicate p)
{
	return FilterRange(r, FindAll(r, p));
}
