#ifndef _NEW_DELETE_FORM_H_
#define _NEW_DELETE_FORM_H_

#include<new>
#include<assert.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
//std_new_delete
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#define _RESTRICT_ __declspec(restrict)
#endif

#ifndef _MSC_VER
static inline void *_aligned_malloc( size_t size, size_t alignment )
 {
    void *p;
    int ret = posix_memalign( &p, alignment, size );
    return (ret == 0) ? p : 0;
 }

 static inline void _aligned_free( void* ptr )
 {
 	free( ptr );
 }

#define _RESTRICT_ __restrict

#endif


class std_new_delete
{
public:

	//new delete
	//__declspec(restrict) static void *operator new(size_t _Size) throw(std::bad_alloc)
	_RESTRICT_ static void *operator new(size_t _Size) throw(std::bad_alloc)
	{
		return ::operator new(_Size);
	}

	//__declspec(restrict) static void *operator new[](size_t _Size) throw(std::bad_alloc)
	_RESTRICT_ static void *operator new[](size_t _Size) throw(std::bad_alloc)
	{
		return ::operator new[](_Size);
	}
	static void operator delete(void *_pMemory) throw()
	{
		::operator delete(_pMemory);
	}
	static void operator delete[](void *_pMemory) throw()
	{
		::operator delete[](_pMemory);
	}

	//nothrow new delete
	//__declspec(restrict) static void *operator new(std::size_t size, const std::nothrow_t &nt) throw()
	_RESTRICT_ static void *operator new(std::size_t size, const std::nothrow_t &nt) throw()
	{
		return ::operator new(size, nt);
	}
	
	//__declspec(restrict) static void *operator new[](std::size_t size, const std::nothrow_t &nt) throw()
	_RESTRICT_ static void *operator new[](std::size_t size, const std::nothrow_t &nt) throw()
	{
		return ::operator new[](size, nt);
	}
	static void operator delete(void *_pMemory, const std::nothrow_t&) throw()
	{
		::operator delete(_pMemory);
	}
	static void operator delete[](void *_pMemory, const std::nothrow_t&) throw()
	{
		::operator delete[](_pMemory);
	}
	
	//placement new delete
	//__declspec(restrict) static void *operator new(size_t size, void *_Where) throw()
	_RESTRICT_ static void *operator new(size_t size, void *_Where) throw()
	{
		return ::operator new(size, _Where);
	}
	
	//__declspec(restrict) static void *operator new[](size_t size, void *_Where) throw()
	_RESTRICT_ static void *operator new[](size_t size, void *_Where) throw()
	{
		return ::operator new[](size, _Where);
	}
	static void operator delete(void *_pMemory, void *_Where) throw()
	{
		::operator delete(_pMemory, _Where);
	}
	static void operator delete[](void *_pMemory, void *_Where) throw()
	{
		::operator delete[](_pMemory, _Where);
	}
	
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//aligned_new_delete
///////////////////////////////////////////////////////////////////////////////////////////////////

template<size_t _Alignment> class aligned_new_delete
{
public:

	//new delete
	//__declspec(restrict) static void *operator new(size_t _Size) throw(std::bad_alloc)
	_RESTRICT_ static void *operator new(size_t _Size) throw(std::bad_alloc)
	{
		while(true){
			void *ptr = _aligned_malloc(_Size, _Alignment);
			if(ptr != nullptr) return ptr;
			std::new_handler handler = std::set_new_handler(nullptr);
			std::set_new_handler(handler);
			if(handler == nullptr) throw std::bad_alloc();
			handler();
		}
	}
	
	//__declspec(restrict) static void *operator new[](size_t _Size) throw(std::bad_alloc)
	_RESTRICT_ static void *operator new[](size_t _Size) throw(std::bad_alloc)
	{
		return aligned_new_delete<_Alignment>::operator new(_Size);
	}
	static void operator delete(void *_Memory) throw()
	{
		_aligned_free(_Memory);
	}
	static void operator delete[](void *_Memory) throw()
	{
		_aligned_free(_Memory);
	}

	//nothow new delete
	//__declspec(restrict) static void *operator new(size_t _Size, const std::nothrow_t&) throw()
	_RESTRICT_ static void *operator new(size_t _Size, const std::nothrow_t&) throw()
	{
		return _aligned_malloc(_Size, _Alignment);
	}
	//__declspec(restrict) static void *operator new[](size_t _Size, const std::nothrow_t&) throw()
	_RESTRICT_ static void *operator new[](size_t _Size, const std::nothrow_t&) throw()
	{
		return _aligned_malloc(_Size, _Alignment);
	}
	static void operator delete(void *_Memory, const std::nothrow_t&) throw()
	{
		_aligned_free(_Memory);
	}
	static void operator delete[](void *_Memory, const std::nothrow_t&) throw()
	{
		_aligned_free(_Memory);
	}

	//placement new delete
	//__declspec(restrict) static void *operator new(size_t, void *_Where) throw()
	_RESTRICT_ static void *operator new(size_t, void *_Where) throw()
	{
		assert(reinterpret_cast<size_t>(_Where) % _Alignment == 0);
		return _Where;
	}
	//__declspec(restrict) static void *operator new[](size_t, void *_Where) throw()
	_RESTRICT_ static void *operator new[](size_t, void *_Where) throw()
	{
		assert(reinterpret_cast<size_t>(_Where) % _Alignment == 0);
		return _Where;
	}
	static void operator delete(void*, void*) throw()
	{
	}
	static void operator delete[](void*, void*) throw()
	{
	}

};

///////////////////////////////////////////////////////////////////////////////////////////////////

//#pragma warning(default : 4127) //èåèéÆÇ™íËêî
//#pragma warning(default : 4290) //ó·äOéwíË

#undef _RESTRICT_

#endif
