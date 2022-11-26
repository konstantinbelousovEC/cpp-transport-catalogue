#pragma once

#include <iostream>
#include <vector>
#include <deque>
#include "json.h"

namespace json {
    class Builder;
    class DictItemContext;
    class ArrayItemContext;
    class KeyItemContext;
    class ValueAfterArrayContext;

    class ItemContext {
    public:
        ItemContext(Builder& builder) : builder_(builder) {};
        ValueAfterArrayContext Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();
    protected:
        Builder& builder_;
    };

    class ValueAfterKeyContext : public ItemContext {
    public:
        KeyItemContext Key(std::string key);
        ValueAfterArrayContext Value(Node::Value value) = delete;
        Builder& EndArray() = delete;
        DictItemContext StartDict() = delete;
        ArrayItemContext StartArray() = delete;
    };

    class ValueAfterArrayContext : public ItemContext {
    public:
        Builder& EndDict() = delete;
    };

    class KeyItemContext : public ItemContext {
    public:
        ValueAfterKeyContext Value(Node::Value value);
        Builder& EndDict() = delete;
        Builder& EndArray() = delete;
    };
    class DictItemContext : public ItemContext {
    public:
        KeyItemContext Key(std::string key);
        ValueAfterArrayContext Value(Node::Value value) = delete;
        Builder& EndArray() = delete;
        DictItemContext StartDict() = delete;
        ArrayItemContext StartArray() = delete;
    };
    class ArrayItemContext : public ItemContext {
    public:
        Builder& EndDict() = delete;
    };

    class Builder {
    public:
        Builder& Key(std::string key);
        Builder& Value(Node::Value value);
        DictItemContext StartDict() {
            StartData(Dict{});
            return {*this};
        }
        ArrayItemContext StartArray() {
            StartData(Array{});
            return {*this};
        }
        Builder& EndDict() {
            EndData("Dict");
            return *this;
        }
        Builder& EndArray() {
            EndData("Array");
            return *this;
        }
        json::Node Build() {
            if (nodes_stack_.empty() && root_ != nullptr) {
                return root_;
            } else {
                throw std::logic_error("calling build when object is not ready");
            }
        }

    private:
        template<typename T>
        void StartData(T obj);

        void EndData(const std::string& type);

        Node root_ = nullptr;
        std::vector<Node*> nodes_stack_;
        std::deque<Node> nodes_;
    };

    template<typename T>
    void Builder::StartData(T obj) {
        std::string str = std::is_same<T, Array>::value ? "Array" : "Dict";
        if (root_ != nullptr) throw std::logic_error("calling Start" + str + "-method for ready object");
        if (nodes_stack_.empty() || nodes_stack_.back()->IsArray() || nodes_stack_.back()->IsString()) {
            nodes_.emplace_back(obj);
            nodes_stack_.push_back(&nodes_.back());
        } else {
            throw std::logic_error("calling Start" + str + "-method in wrong place");
        }
    }
}