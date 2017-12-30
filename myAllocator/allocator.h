#pragma once

/**********����Release x64������************/
#include<iostream>

#define NumOfList 16
#define Block 256
#define Max_size  Block*NumOfList
#define makeup(bytes) (bytes + Block) & ~(Block - 1) //������ڴ治��block�ı����Ļ����ϵ���block�ı���
#define index(size) ((size + Block - 1) / Block - 1) //�����������ݿ��С�ҵ�freelist���±�
using namespace std;

union node //freelist�ڵ�
{
	node* next;
	char data[1];
};

typedef struct pool {
	 char* first;
	 char *last;
	 size_t size;
}poolptr; //�ڴ��ָ��

static poolptr pool;  //����һ���ڴ��
node* free_list[NumOfList] = { 0 }; //����freelists����k��freelistָ��k*block��С���ڴ�


static char* pool_alloc(size_t size, int& num); //���ڴ������size*num��С���ڴ�
 static void* Refill(size_t n); //���·���ռ�



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
		if (size > Max_size) //�����max_size����ÿ⺯������
		{
			::operator delete (_Ptr);
		}
		else //������յ���������
		{
			temp_list = free_list + index(size);
			t->next = *temp_list;//�ڿ�������ͷ������t
			*temp_list = t;

		}
	}
	_DECLSPEC_ALLOCATOR pointer allocate(size_type _Count)
	{
		node** temp_list;
		node* result=NULL;
		size_type size = sizeof(size_type)*_Count;
		if (size > Max_size)//�����ڴ�����ڴ������ڴ�
		{
			return reinterpret_cast<pointer>(operator new (size));
		}
	
		temp_list = free_list + index(size);//Ѱ��һ����������
		result = *temp_list;
		if (result == NULL) //�ÿ�������û�п������ݿ飬�����·���ռ�,���������·���Ŀռ�
		{
			void *r = Refill(makeup(size));
			return reinterpret_cast<pointer>(r);
		}
		*temp_list = result->next;//ȡ�����п飬����ָ����һ����
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
	char * chunk = pool_alloc(n, num); //���ڴ��num����СΪn���ڴ��

	temp_list = free_list + index(n);
	result = (node*)chunk;//�ѵ�һ���ڴ��������ߣ������Ĳ��뵽free list
	*temp_list = (node*)(chunk + n); //ָ��ڶ����ڴ��
	next = (node*)(chunk + n);
	int i = 1;
	do
	{
		current = next;
		next = (node*)((char*)next + n); //n��Ϊһ�飬ָ����һ��
		if (num != i + 1) //���������һ��
		{
			current->next = next;//�����������ݿ�
		}
		else
		{
			current->next = NULL; //ָ���
			break;
		}
		i++;

	} while (true);

	return result;
}

static char* pool_alloc(size_t size, int& num) //���ڴ������size*num��С���ڴ�
{
	char *result;
	size_t require = size*num; //�������ڴ�
	size_t left = pool.last - pool.first; //��ǰ�ڴ�ص��ڴ�
	size_t more;
	node** temp_list;
	node** temp_list2;
	
	if (left >= require) //���пռ����
	{
		result = pool.first; //ֱ�ӷ���
		pool.first = pool.first + require; //�ڴ�ص�ָ�����
		return result;
	}

	else //���пռ䲻��������
	{
		more = 2 * require + makeup(pool.size / 16);
		if (left > 0) //��ʣ��Ŀ�ŵ�freelist
		{
			temp_list = free_list + index(left); //�ŵ���ʱ������
			reinterpret_cast<node*>(pool.first)->next = *temp_list; //���뵽freelist
			*temp_list = ((node*)pool.first);
		}
		pool.first = (char*)malloc(more);
		pool.size += more;
		pool.last = pool.first + more;

		return pool_alloc(size, num);
	}

}
