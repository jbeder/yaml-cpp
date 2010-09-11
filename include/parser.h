#pragma once

#ifndef PARSER_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define PARSER_H_62B23520_7C8E_11DE_8A39_0800200C9A66


#include "graphbuilder.h"
#include "noncopyable.h"
#include <ios>
#include <memory>

namespace YAML
{
	struct Directives;
	struct Mark;
	struct Token;
	class EventHandler;
	class GraphBuilderInterface;
	class Node;
	class Scanner;

	class Parser: private noncopyable
	{
	public:
		Parser();
		Parser(std::istream& in);
		~Parser();

		operator bool() const;

		void Load(std::istream& in);
		bool HandleNextDocument(EventHandler& eventHandler);
		
		void *BuildNextDocumentGraph(GraphBuilderInterface& graphBuilder);
		template <class Builder>
		typename Builder::Node *BuildNextDocumentGraph(Builder& graphBuilder)
		{
			GraphBuilder<Builder> wrapper(graphBuilder); // Must be lvalue to make C++ happy
			return static_cast<typename Builder::Node *>(
				BuildNextDocumentGraph(wrapper.AsBuilderInterface())
			);
		}
		
		bool GetNextDocument(Node& document);
		void PrintTokens(std::ostream& out);

	private:
		void ParseDirectives();
		void HandleDirective(const Token& token);
		void HandleYamlDirective(const Token& token);
		void HandleTagDirective(const Token& token);
		
	private:
		std::auto_ptr<Scanner> m_pScanner;
		std::auto_ptr<Directives> m_pDirectives;
	};
}

#endif // PARSER_H_62B23520_7C8E_11DE_8A39_0800200C9A66
