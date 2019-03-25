template <class Type>
bool List<Type>::empty()
{
	if(first_node == 0)
		return true;
	else
		return false;
}

template <class Type>
bool List<Type>::pop_back()
{
	Type * temp;
	if(empty())
		return false;
	else if(first_node == last_node){
		delete first_node;
		first_node= 0;
		last_node= 0;
	}
	else{
		temp= last_node->previous;
		delete last_node;
		last_node= temp;
		last_node->next= 0;
	}
}

template <class Type>
void List<Type>::push_back(
	const Type& element)
{
	if(empty()){
		last_node= new node<Type>(element);
		first_node= last_node;
		last_node->next= 0;
		last_node->previous= 0;
	}
	else{
		last_node->next= new node<Type>(element);
		last_node->next->previous= last_node;
		last_node= last_node->next;
		last_node->next= 0;
	}
}

template <class Type>
bool List<Type>::move_next()
{
	if(current_node == last_node)
		return false;
	else
		current_node= current_node->next;
	return true;
}

template <class Type>
bool List<Type>::move_previous()
{
	if(current_node == first_node)
		return false;
	else
		current_node= current_node->previous;
	return true;
}

template <class Type>
Type& List<Type>::get_current()
{
	return current_node->data;
}

template <class Type>
void List<Type>::iterator_begin()
{
	current_node= first_node;
}

template <class Type>
void List<Type>::remove_current()
{
	if(current_node == 0)
		return;
	if(size() > 1){
		if(last_node == current_node){
			last_node= last_node->previous;
			last_node->next= 0;
		}
		if(first_node == current_node){
			first_node= first_node->next;
			first_node->previous= 0;
		}
		else{
			current_node->next->previous= current_node->previous;
			current_node->previous->next= current_node->next;
		}
		delete current_node;
		current_node= 0;
	}
	else if(size() == 1){
		delete current_node;
	}
}

template <class Type>
int List<Type>::size()
{
	if(empty())
		return 0;
	else{
		int count= 1;
		node<Type> * node_ptr;
		node_ptr= first_node;
		while(node_ptr->next != 0){
			node_ptr= node_ptr->next;
			count++;
		}
		return count;
	}
}