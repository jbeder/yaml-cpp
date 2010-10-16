#include "nodebuilder.h"
#include "mark.h"
#include "node.h"
#include "nodeproperties.h"
#include <cassert>

namespace YAML
{
	NodeBuilder::NodeBuilder(Node& root): m_root(root), m_initializedRoot(false)
	{
		m_root.Clear();
	}
	
	NodeBuilder::NodeBuilder(const NodeBuilder& o): m_root(o.m_root), m_initializedRoot(o.m_initializedRoot)
	{
	}
	
	NodeBuilder::~NodeBuilder()
	{
	}

	Node *NodeBuilder::NewNull(const std::string& tag, Node *pParent)
	{
		(void)pParent;
		Node *pNode = NewNode();
		pNode->InitNull(tag);
		return pNode;
	}

	Node *NodeBuilder::AnchorReference(const Mark& mark, Node *pNode)
	{
		Node *pAlias = NewNode();
		pAlias->InitAlias(mark, *pNode);
		return pAlias;
	}

	Node *NodeBuilder::NewScalar(const Mark& mark, const std::string& tag, Node *pParent, const std::string& value)
	{
		(void)pParent;
		Node *pNode = NewNode();
		pNode->Init(CT_SCALAR, mark, tag);
		pNode->SetData(value);
		return pNode;
	}

	NodeBuilder::Sequence *NodeBuilder::NewSequence(const Mark& mark, const std::string& tag, Node *pParent)
	{
		(void)pParent;
		Node *pNode = NewNode();
		pNode->Init(CT_SEQUENCE, mark, tag);
		return pNode;
	}

	void NodeBuilder::AppendToSequence(Sequence *pSequence, Node *pNode)
	{
		std::auto_ptr<Node> apNode(m_unlinked.pop(pNode));
		assert(apNode.get());
		pSequence->Append(apNode);
	}

	NodeBuilder::Map *NodeBuilder::NewMap(const Mark& mark, const std::string& tag, Node* pParent)
	{
		(void)pParent;
		Node *pNode = NewNode();
		pNode->Init(CT_MAP, mark, tag);
		return pNode;
	}

	void NodeBuilder::AssignInMap(Map *pMap, Node *pKeyNode, Node *pValueNode)
	{
		std::auto_ptr<Node> apKeyNode(m_unlinked.pop(pKeyNode));
		std::auto_ptr<Node> apValueNode(m_unlinked.pop(pValueNode));
		assert(apKeyNode.get() && apValueNode.get());
		pMap->Insert(apKeyNode, apValueNode);
	}
	
	Node* NodeBuilder::NewNode()
	{
		if(!m_initializedRoot) {
			m_initializedRoot = true;
			return &m_root;
		}
		
		std::auto_ptr<Node> pNode(new Node);
		Node* pResult = pNode.get();
		// Save the pointer in a collection that will free it on exception
		m_unlinked.push(pNode);
		return pResult;
	}
}
