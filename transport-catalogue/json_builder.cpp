#include "json_builder.h"

namespace json {

    Builder& Builder::Key(std::string key) {
        if (root_ != nullptr) throw std::logic_error("calling Key-method for ready object");
        if (nodes_stack_.back()->IsDict()) {
            Node::Value str{key};
            nodes_.emplace_back(std::move(str));
            nodes_stack_.push_back(&nodes_.back());
        } else {
            throw std::logic_error("calling Key-method in wrong place");
        }
        return *this;
    }

    Builder& Builder::Value(Node::Value value) {
        if (root_ != nullptr) throw std::logic_error("calling Value-method for ready object");
        if (root_ == nullptr && nodes_stack_.empty()) {
            root_ = std::move(value);
        } else if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->AsArray().emplace_back(std::move(value));
        } else if (nodes_stack_.back()->IsString()) {
            Node& node_ref =*nodes_stack_.back();
            nodes_stack_.pop_back();
            nodes_stack_.back()->AsDict().insert({node_ref.AsString(), std::move(value)});
        } else {
            throw std::logic_error("calling Value-method in wrong place");
        }
        return *this;
    }

    void Builder::EndData(const std::string& type) {
        if (nodes_stack_.empty()) throw std::logic_error("calling End" + type + "-method for ready or empty object");
        if (type == "Dict" ? nodes_stack_.back()->IsDict() : nodes_stack_.back()->IsArray()) {
            Node& node_ref = *nodes_stack_.back();
            nodes_stack_.pop_back();
            if (nodes_stack_.empty()) {
                root_ = std::move(node_ref);
            } else if (nodes_stack_.back()->IsArray()) {
                nodes_stack_.back()->AsArray().emplace_back(std::move(node_ref));
            } else if (nodes_stack_.back()->IsString()) {
                Node& str_node_ref = *nodes_stack_.back();
                nodes_stack_.pop_back();
                nodes_stack_.back()->AsDict().insert({str_node_ref.AsString(), std::move(node_ref)});
            }
        } else {
            throw std::logic_error("calling End" + type + "-method in wrong place");
        }
    }

    Builder& ItemContext::EndDict() {
        builder_.EndDict();
        return builder_;
    }
    Builder& ItemContext::EndArray() {
        builder_.EndArray();
        return builder_;
    }
    DictItemContext ItemContext::StartDict() {
        builder_.StartDict();
        return {*this};
    }
    ArrayItemContext ItemContext::StartArray() {
        builder_.StartArray();
        return {*this};
    }
    ValueAfterArrayContext ItemContext::Value(Node::Value value) {
        builder_.Value(std::move(value));
        return {*this};
    }
    ValueAfterKeyContext KeyItemContext::Value(Node::Value value) {
        builder_.Value(std::move(value));
        return {*this};
    }
    KeyItemContext ValueAfterKeyContext::Key(std::string key) {
        builder_.Key(std::move(key));
        return {*this};
    }
    KeyItemContext DictItemContext::Key(std::string key) {
        builder_.Key(std::move(key));
        return {*this};
    }
}