#pragma once

#ifndef NODEBUILDER_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define NODEBUILDER_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#include <map>
#include <memory>
#include <stack>
#include <string>

#include "ptr_stack.h"

namespace YAML
{
	class Node;
	class Mark;
	
	class NodeBuilder
	{
	public:
		typedef YAML::Node Node;
		typedef YAML::Node Map;
		typedef YAML::Node Sequence;
		
		explicit NodeBuilder(Node& root);
		NodeBuilder(const NodeBuilder& o);
		virtual ~NodeBuilder();

		Node *NewNull(const std::string& tag, Node *pParent);
		Node *AnchorReference(const Mark& mark, Node *pNode);
		Node *NewScalar(const Mark& mark, const std::string& tag, Node *pParent, const std::string& value);
		
		Sequence *NewSequence(const Mark& mark, const std::string& tag, Node *pParent);
		void AppendToSequence(Sequence *pSequence, Node *pNode);
		void SequenceComplete(Sequence *pSequence) {(void)pSequence;}
		
		Map *NewMap(const Mark& mark, const std::string& tag, Node *pParent);
		void AssignInMap(Map *pMap, Node *pKeyNode, Node *pValueNode);
		void MapComplete(Map *pMap) {(void)pMap;}
		
	private:
		Node* NewNode();
		
	private:
		Node& m_root;
		bool m_initializedRoot;
		
		ptr_stack<Node> m_unlinked;
	};
}

#endif // NODEBUILDER_H_62B23520_7C8E_11DE_8A39_0800200C9A66

