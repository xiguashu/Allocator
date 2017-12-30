#pragma once

/**********请在Release x64下运行************/
#include<iostream>

#define NumOfList 16
#define Block 256
#define Max_size  Block*NumOfList
#define makeup(bytes) (bytes + Block) & ~(Block - 1) //申请的内存不是block的倍数的话，上调至block的倍数
#define index(size) ((size + Block - 1) / Block - 1) //根据申请数据块大小找到freelist的下标
using namespace std;

union node //freelist节点
{
	node* next;
	char data[1];
};

typedef struct pool {
	 char* first;
	 char *last;
	 size_t size;
}poolptr; //内存池指针

static poolptr pool;  //创建一个内存池
node* free_list[NumOfList] = { 0 }; //创建freelists，第k个freelist指向k*block大小的内存


static char* pool_alloc(size_t size, int& num); //从内存池申请size*num大小的内存
 static void* Refill(size_t n); //重新分配空间



template <class T>
class myAllocator
{

public:
	typedef void _Not_user_specialized;
	typedef T value_type;
	typedef value_type *pointer;
	typedef const value_type *const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef true_type propagate_on_container_move_assignment;
	typedef true_type is_always_equal;


	myAllocator() 
	{
	
	}

	~myAllocator()
	{
	}
	template <class _Uty> myAllocator(const myAllocator<_Uty>& refAllocator) noexcept: myAllocator()
	{

	};
	template <class _Uty> struct rebind
	{
		typedef myAllocator<_Uty> other;
	};
	pointer address(reference _Val) const _NOEXCEPT
	{
		return &_Val;
	}
	const_pointer address(const_reference _Val) const _NOEXCEPT
	{
		return &_Val;
	}
	void deallocate(pointer _Ptr, size_type _Count)
	{
		node *t = (node*)_Ptr;
		node** temp_list;
		size_type size = sizeof(size_type)*_Count;
		if (size > Max_size) //块大于max_size则调用库函数回收
		{
			::operator delete (_Ptr);
		}
		else //否则回收到空闲链表
		{
			temp_list = free_list + index(size);
			t->next = *temp_list;//在空闲链表头部插入t
			*temp_list = t;

		}
	}
	_DECLSPEC_ALLOCATOR pointer allocate(size_type _Count)
	{
		node** temp_list;
		node* result=NULL;
		size_type size = sizeof(size_type)*_Count;
		if (size > Max_size)//申请内存大于内存池最大内存
		{
			return reinterpret_cast<pointer>(operator new (size));
		}
	
		temp_list = free_list + index(size);//寻找一个空闲链表
		result = *temp_list;
		if (result == NULL) //该空闲链表没有可用数据块，则重新分配空间,并返回重新分配的空间
		{
			void *r = Refill(makeup(size));
			return reinterpret_cast<pointer>(r);
		}
		*temp_list = result->next;//取出空闲块，链表指向下一个块
		return reinterpret_cast<pointer>(result);

	}

	template<class _Uty> void destroy(_Uty *_Ptr)
	{
		_Ptr->~_Uty();
	}


	template<class _Objty, class... _Types>
	void construct(_Objty *_Ptr, _Types&&... _Args)
	{
		new(_Ptr) _Objty(forward<_Types>(_Args)...);
	}
};



static void* Refill(size_t n)
{
	int num = 10;
	node **temp_list;
	node *result, *current, *next;
	char * chunk = pool_alloc(n, num); //从内存池num个大小为n的内存块

	temp_list = free_list + index(n);
	result = (node*)chunk;//把第一个内存块给申请者，其他的插入到free list
	*temp_list = (node*)(chunk + n); //指向第二个内存块
	next = (node*)(chunk + n);
	int i = 1;
	do
	{
		current = next;
		next = (node*)((char*)next + n); //n个为一块，指向下一块
		if (num != i + 1) //还不是最后一块
		{
			current->next = next;//往链表后加数据块
		}
		else
		{
			current->next = NULL; //指向空
			break;
		}
		i++;

	} while (true);

	return result;
}

static char* pool_alloc(size_t size, int& num) //从内存池申请size*num大小的内存
{
	char *result;
	size_t require = size*num; //申请总内存
	size_t left = pool.last - pool.first; //当前内存池的内存
	size_t more;
	node** temp_list;
	node** temp_list2;
	
	if (left >= require) //现有空间充足
	{
		result = pool.first; //直接分配
		pool.first = pool.first + require; //内存池的指针后移
		return result;
	}

	else //现有空间不够则申请
	{
		more = 2 * require + makeup(pool.size / 16);
		if (left > 0) //把剩余的块放到freelist
		{
			temp_list = free_list + index(left); //放到临时链表中
			reinterpret_cast<node*>(pool.first)->next = *temp_list; //插入到freelist
			*temp_list = ((node*)pool.first);
		}
		pool.first = (char*)malloc(more);
		pool.size += more;
		pool.last = pool.first + more;

		return pool_alloc(size, num);
	}

}
