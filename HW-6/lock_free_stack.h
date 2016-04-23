#ifndef H_LOCK_FREE_STACK
#define H_LOCK_FREE_STACK

#include <iostream>
#include <atomic>
#include <memory>

using namespace std;

template <class T>
class lock_free_stack {
	struct node	{
		std::shared_ptr<T> data;
		std::atomic<node*> next;
		node (T const & _data): data( std::make_shared<T>(_data) ), next(nullptr) {
		}
	};
private:
	std::atomic<size_t> counter;
	std::atomic<node*> head;
	std::atomic<node*> list_to_delete;

	void try_delete(node* node_to_delete) {
		if (counter == 1) {
			node* delete_now = list_to_delete.exchange(nullptr);
			if (!--counter) {
				while (delete_now) {
					node* old = delete_now;
					delete_now = delete_now->next;
					delete old;
				}
			} else {
				if (delete_now) {
					node* last = delete_now;
					while (last->next) {
						last = last->next;
					}
					node* old_list = list_to_delete.load();
					last->next.store(old_list);

					while( !list_to_delete.compare_exchange_weak(old_list, delete_now) ) {
						last->next.store(old_list);
					};
				}
			}
			delete node_to_delete;
		} else {
			node* old_list = list_to_delete.load();
			node_to_delete->next.store(old_list);

			while( !list_to_delete.compare_exchange_weak(old_list, node_to_delete) ) {
				node_to_delete->next.store(old_list);
			};
			--counter;
		}
		std::cout << "tD" << std::endl;
		return;
	}

public:
	lock_free_stack(): counter(0), head(nullptr), list_to_delete(nullptr) {
		cout << "c-tor" << endl;
	}
	~lock_free_stack() {
		while (head) {
			node* curr_head = head.load();
			head.compare_exchange_weak(curr_head, curr_head->next);
			try_delete(curr_head);
		}
	}
	void push(T const & _data) {
		cout << "push" << endl;
		node* new_node = new node(_data);
		node* old_head = head.load();
		new_node->next.store(old_head);
		cout << "b" << endl;
		while (!head.compare_exchange_weak(old_head, new_node)) {
		    new_node->next.store(old_head);
		}
		cout << "p1" << endl;
		return;
	}
	std::shared_ptr<T> pop() {
		cout << "pop" << endl;
		counter++;

		node* curr_head = head.load();
		if ( !curr_head ) {
			cout << "p2" << endl;
			return std::shared_ptr<T>();
		}
		while( !head.compare_exchange_weak(curr_head, curr_head->next) ) {
		};
		std::shared_ptr<T> result = curr_head->data;
		if (curr_head) {
			try_delete(curr_head);
		}
		cout << "p3" << endl;
		return result;
	}
};

#endif