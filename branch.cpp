#include <vector>
#include <map>

class Mutex;
class Node;

class Node
{
public:
  virtual ~Node();
  Node* OnMutexLock(Mutex*);
  virtual Node* OnMutexUnlock(Mutex*) = 0;

  struct MutexList
  {
    Mutex*    mutex;
    MutexList* next;
  };

  Node* Branch(MutexList* list);
  virtual Node* FindNode(MutexList* list, Mutex* mutex) = 0;

protected:
  std::map<Mutex*, Node*> nodes_;
};

class RootNode: public Node
{
public:
  Node* OnMutexUnlock(Mutex*) override;
  Node* FindNode(MutexList* list, Mutex* mutex) override;
};

class TreeNode: public Node
{
public:
  TreeNode(Node* parent, Mutex* mutex);
  TreeNode(Node* parent, MutexList* list);
  Node* OnMutexUnlock(Mutex*) override;
  Node* FindNode(MutexList* list, Mutex* mutex) override;

private:
  Node* parent_ {};
  Mutex* mutex_;
};

Node::~Node()
{
  for(auto pair: nodes_)
  {
    delete pair.second;
  }
}

Node* Node::OnMutexLock(Mutex* mutex)
{
  Node* node;

  auto node_pair = nodes_.find(mutex);
  if(node_pair != std::end(nodes_))
  {
    node = node_pair->second;
  }
  else
  {
    node = new TreeNode(this, mutex);
    nodes_[mutex] = node;
  }

  return node;
}

Node* Node::Branch(MutexList* list)
{
  Node* node;

  auto node_pair = nodes_.find(list->mutex);
  if(node_pair == std::end(nodes_))
  {
    node = new TreeNode(this, list);
    nodes_[list->mutex] = node;
  }
  else if(list->next)
  {
    node = node_pair->second->Branch(list->next);
  }
  else
  {
    node = this;
  }

  return node;
}

Node* RootNode::OnMutexUnlock(Mutex*)
{
  abort();
}

Node* RootNode::FindNode(MutexList*, Mutex*)
{
  abort();
}

TreeNode::TreeNode(Node* parent, Mutex* mutex):
  parent_(parent),
  mutex_(mutex)
{}

TreeNode::TreeNode(Node* parent, MutexList* list):
  TreeNode(parent, list->mutex)
{
  list = list->next;

  if(list)
  {
    nodes_[list->mutex] = new TreeNode(this, list);
  }
}

Node* TreeNode::OnMutexUnlock(Mutex* mutex)
{
  Node* node;

  if(mutex == mutex_)
  {
    node = parent_;
  }
  else
  {
    MutexList list {mutex_, nullptr};
    node = parent_->FindNode(&list, mutex);
  }

  return node;
}

Node* TreeNode::FindNode(MutexList* next, Mutex* mutex)
{
  Node* node;

  if(mutex == mutex_)
  {
    node = parent_->Branch(next);
  }
  else
  {
    MutexList list {mutex_, next};
    node = parent_->FindNode(&list, mutex);
  }

  return node;
}
