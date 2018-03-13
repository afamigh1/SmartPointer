#ifndef SHAREDPTR_HPP
#define SHAREDPTR_HPP
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <mutex>
#include <string>
#include <cstddef>
#include <typeinfo>

/**	========= NOTES =========	
#	Single & means copy (copy constructor, copy assignment)
#	Double && means move (move constructor, move assignment)
#							
#															
**/

using namespace std;

namespace cs540 {

	mutex thread_lock;
	
	class VirtualBase;
	
	template <typename T> 
	class DeleteObject;

	/*
			The smart pointer points to an object of type T
			T may refer to a const-qualified type
	*/
	template <typename T> 
	class SharedPtr {
		public:
			/*
				Data Members for the SharedPtr class
			*/
			T* smart_ptr;
			unsigned int* refcount;
			VirtualBase* d;
			
			typedef T object_type;
			
			/*
				Public Member Functions
			*/
			SharedPtr() : 
				smart_ptr(nullptr), 
				refcount(nullptr),
				d(nullptr) { }
				
			template <typename U> 
			explicit SharedPtr(U *obj_ptr) { 
				if(obj_ptr == nullptr) {
					smart_ptr = nullptr;
					refcount = nullptr;
					d = nullptr;
				} else {
					smart_ptr = obj_ptr;
					refcount = new unsigned int(1);
					d = new DeleteObject<U>(obj_ptr);
				}
			}
			
			SharedPtr(const SharedPtr &other_ptr) {
				//std::cout << "copy constructor called" << std::endl;
				lock_guard<mutex> lock(thread_lock);
				
				smart_ptr = other_ptr.smart_ptr;
				refcount = other_ptr.refcount;
				if(refcount != nullptr) (*(refcount))++;
				d = other_ptr.d;				
			}
			
			template <typename U> 
			SharedPtr(const SharedPtr<U> &other_ptr) {
				lock_guard<mutex> lock(thread_lock);
				
				smart_ptr = other_ptr.smart_ptr;
				refcount = other_ptr.refcount;
				if(refcount != nullptr) (*(refcount))++;
				d = other_ptr.d;
			}
			
			template <typename U> 
			SharedPtr(const SharedPtr<U> &p, T* obj) {
				lock_guard<mutex> lock(thread_lock);
				
				if(obj != nullptr){
					refcount = p.refcount;
					(*(refcount))++;
					smart_ptr = obj;
					d = p.d;
				} else { 
					refcount = nullptr;
					smart_ptr = nullptr;
					d = nullptr;
				}
			}	
			
			/*	Move Constructor	*/
			/*	good documentation on https://msdn.microsoft.com/en-us/library/dd293665.aspx */
			SharedPtr(SharedPtr &&other_ptr) :
						smart_ptr(nullptr), 
						refcount(nullptr),
						d(nullptr) {
				//std::cout << "move assignment constructor" << std::endl;
				lock_guard<mutex> lock(thread_lock);
				
				smart_ptr = other_ptr.smart_ptr;
				refcount = other_ptr.refcount;
				d = other_ptr.d;
				
				other_ptr.smart_ptr = nullptr;
				other_ptr.refcount = nullptr;	
				other_ptr.d = nullptr;			
			}
			
			template <typename U> 
			SharedPtr(SharedPtr<U> &&other_ptr) :
									smart_ptr(nullptr), 
									refcount(nullptr),
									d(nullptr) {
				
				lock_guard<mutex> lock(thread_lock);
				
				smart_ptr = other_ptr.smart_ptr;
				refcount = other_ptr.refcount;
				d = other_ptr.d;
				
				other_ptr.smart_ptr = nullptr;
				other_ptr.refcount = nullptr;
				other_ptr.d = nullptr;
			}
			
			SharedPtr &operator=(const SharedPtr &other_ptr) {
				//std::cout << "copy assignment operator" << std::endl;
				lock_guard<mutex> lock(thread_lock);
				
				if(this == &other_ptr) { return *this; }	//handles self assignment
				else {
					if(refcount != nullptr) {
						(*refcount)--;
						if(*refcount == 0) {
							delete d;
							delete refcount;
						}
					}
					
					d = other_ptr.d;
					refcount = other_ptr.refcount;
					if(refcount != nullptr) (*refcount)++;
					smart_ptr = other_ptr.smart_ptr;
					
					return *this;
				}
			}
			
			template <typename U> 
			SharedPtr<T> &operator=(const SharedPtr<U> &other_ptr) {
				lock_guard<mutex> lock(thread_lock);
				
				if(refcount != nullptr) {
					(*refcount)--;
					if(*refcount == 0) {
						delete d;
						delete refcount;
					}
				}
				
				d = other_ptr.d;
				refcount = other_ptr.refcount;
				if(refcount != nullptr) (*refcount)++;
				smart_ptr = other_ptr.smart_ptr;
				
				return *this;
			}
			
			SharedPtr &operator=(SharedPtr &&other_ptr) {
				//std::cout << "move assignment operator" << std::endl;
				lock_guard<mutex> lock(thread_lock);
				
				smart_ptr = other_ptr.smart_ptr;
				refcount = other_ptr.refcount;
				d = other_ptr.d;
				
				other_ptr.smart_ptr = nullptr;
				other_ptr.refcount = nullptr;
				other_ptr.d = nullptr;
				
				return *this;
			}
			
			template <typename U> SharedPtr &operator=(SharedPtr<U> &&other_ptr) {
				lock_guard<mutex> lock(thread_lock);
				
				smart_ptr = other_ptr.smart_ptr;
				refcount = other_ptr.refcount;
				d = other_ptr.d;
				
				other_ptr.smart_ptr = nullptr;
				other_ptr.refcount = nullptr;
				other_ptr.d = nullptr;
				
				return *this;
			}
			
			~SharedPtr() {
				lock_guard<mutex> lock(thread_lock);
				
				if(refcount != nullptr) {
					(*refcount)--;
					if(*refcount == 0) {
						delete refcount;
						delete d;
					}
				
					smart_ptr = nullptr;
					refcount = nullptr;
					d = nullptr;
				}
			}
			
			/*
				Modifiers
			*/
			
			void reset() {
				lock_guard<mutex> lock(thread_lock);
				
				if(refcount != nullptr) {
					(*refcount)--;
					if(*refcount == 0) {
						delete refcount;
						delete d;
					}
				
					smart_ptr = nullptr;
					refcount = nullptr;
					d = nullptr;
				}				
				
			}
			
			template <typename U> 
			void reset(U *obj_ptr) {
				lock_guard<mutex> lock(thread_lock);
				if(refcount != nullptr) {
					(*refcount)--;
					if(*refcount == 0) {
						delete refcount;
						delete d;
					}
				
					if(obj_ptr == nullptr) {
						smart_ptr = nullptr;
						refcount = nullptr;
						d = nullptr;
					} else {
						smart_ptr = obj_ptr;
						refcount = new unsigned int(1);
						d = new DeleteObject<U>(obj_ptr);
					}
				}
			}
			
			
			/*
				Observers
			*/
			
			T *get() const {
				lock_guard<mutex> lock(thread_lock);
				return smart_ptr;
			}
			
			T &operator*() const {
				lock_guard<mutex> lock(thread_lock);
				return *smart_ptr;
			}

			T *operator->() const {
				lock_guard<mutex> lock(thread_lock);
				return smart_ptr;
			}
			
			explicit operator bool() const {
				lock_guard<mutex> lock(thread_lock);
				return (smart_ptr != nullptr) ? true : false;
			}
		
	};
	
	class VirtualBase {
		public:
			virtual ~VirtualBase() { }
	};
	
	template <typename T> 
	class DeleteObject : public VirtualBase {
		public:
			T* object_ptr;
		
			DeleteObject(T* other_ptr) : object_ptr(other_ptr) { }
			
			~DeleteObject() {
				delete object_ptr;
			}
	};
	
	template <typename T1, typename T2>
	bool operator==(const SharedPtr<T1> &other1, const SharedPtr<T2> &other2) {
		return ( other1.smart_ptr == other2.smart_ptr );
	}
	
	template <typename T>
	bool operator==(const SharedPtr<T> &other, std::nullptr_t) {
		return ( other.smart_ptr == nullptr );
	}
	
	template <typename T>
	bool operator==(std::nullptr_t, const SharedPtr<T> &other) {
		return ( other.smart_ptr == nullptr );
	}
	
	template <typename T1, typename T2>
	bool operator!=(const SharedPtr<T1>&other1, const SharedPtr<T2> &other2) {
		return !( other1.smart_ptr == other2.smart_ptr );
	}
	
	template <typename T>
	bool operator!=(const SharedPtr<T> &other, std::nullptr_t) {
		return !( other.smart_ptr == nullptr );
	}
	
	template <typename T>
	bool operator!=(std::nullptr_t, const SharedPtr<T> &other) {
		return !( other.smart_ptr == nullptr );
	}
	
	template <typename T, typename U> 
	SharedPtr<T> static_pointer_cast(const SharedPtr<U> &other){
		return SharedPtr<T>(other, static_cast<typename SharedPtr<T>::object_type*>(other.get()));
	}
	template <typename T, typename U> 
	SharedPtr<T> dynamic_pointer_cast(const SharedPtr<U> &other){
	    	return SharedPtr<T>(other, dynamic_cast<typename SharedPtr<T>::object_type*>(other.get()));
	}
}

#endif
