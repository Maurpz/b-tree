#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>
using namespace std;


template <typename Key, typename Value, int ORDER = 4>
class BPlusTree {
private:
    //*NOdo base
    struct Node {
        bool is_leaf;
        vector<Key> keys;
        Node* parent;

        Node() : is_leaf(false), parent(nullptr) {}
        virtual ~Node() = default;

        bool is_full() const {
            return keys.size() >= ORDER - 1;
        }
    };

    //*nodo hoja
    struct LeafNode : public Node {
        vector<Value> values;
        LeafNode* next;

        LeafNode() : next(nullptr) {
            this->is_leaf = true;
        }
    };

    //*nodo interno
    struct InternalNode : public Node {
        vector<Node*> children;
    };

    Node* root;

    //*metodos auxiliares
    LeafNode* find_leaf(const Key& key) const {
        if (!root) return nullptr;

        Node* current = root;
        while (!current->is_leaf) {
            auto* internal = static_cast<InternalNode*>(current);
            auto it = lower_bound(current->keys.begin(), current->keys.end(), key);
            int index = it - current->keys.begin();
            current = internal->children[index];
        }

        return static_cast<LeafNode*>(current);
    }

    void split_leaf(LeafNode* leaf) {
        LeafNode* new_leaf = new LeafNode();
        int mid = leaf->keys.size() / 2;

        new_leaf->keys.insert(new_leaf->keys.begin(), 
            leaf->keys.begin() + mid, 
            leaf->keys.end());
        new_leaf->values.insert(new_leaf->values.begin(), 
            leaf->values.begin() + mid, 
            leaf->values.end());

        leaf->keys.erase(leaf->keys.begin() + mid, leaf->keys.end());
        leaf->values.erase(leaf->values.begin() + mid, leaf->values.end());

        new_leaf->next = leaf->next;
        leaf->next = new_leaf;

        Key promotion_key = new_leaf->keys[0];
        insert_in_parent(leaf, promotion_key, new_leaf);
    }

    void split_internal(InternalNode* node) {
        InternalNode* new_node = new InternalNode();
        int mid = node->keys.size() / 2;
        Key promotion_key = node->keys[mid];

        new_node->keys.insert(new_node->keys.begin(), 
            node->keys.begin() + mid + 1, 
            node->keys.end());
        node->keys.erase(node->keys.begin() + mid, node->keys.end());

        new_node->children.insert(new_node->children.begin(), 
            node->children.begin() + mid + 1, 
            node->children.end());
        node->children.erase(node->children.begin() + mid + 1, 
            node->children.end());

        for (auto* child : new_node->children) {
            child->parent = new_node;
        }

        insert_in_parent(node, promotion_key, new_node);
    }

    void insert_in_parent(Node* old_node, const Key& key, Node* new_node) {
        if (old_node == root) {
            //*creamos el nodo base
            InternalNode* new_root = new InternalNode();
            new_root->keys.push_back(key);
            new_root->children.push_back(old_node);
            new_root->children.push_back(new_node);
            old_node->parent = new_root;
            new_node->parent = new_root;
            root = new_root;
            return;
        }

        InternalNode* parent = static_cast<InternalNode*>(old_node->parent);
        auto it = lower_bound(parent->keys.begin(), parent->keys.end(), key);
        int index = it - parent->keys.begin();

        parent->keys.insert(it, key);
        parent->children.insert(parent->children.begin() + index + 1, new_node);
        new_node->parent = parent;
        if (parent->is_full()) {
            split_internal(parent);
        }
    }

public:
    BPlusTree() : root(nullptr) {}
    ~BPlusTree() {}

    void insert(const Key& key, const Value& value) {
        //* si esta vacio
        if (!root) {
            root = new LeafNode();
            static_cast<LeafNode*>(root)->keys.push_back(key);
            static_cast<LeafNode*>(root)->values.push_back(value);
            return;
        }

        //*buscamos el nodo para poder insertar
        LeafNode* leaf = find_leaf(key);
        
        auto it = lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
        int index = it - leaf->keys.begin();

        //*insertamos la clave y el valor
        leaf->keys.insert(it, key);
        leaf->values.insert(leaf->values.begin() + index, value);

        //*dividimos si esta lleno
        if (leaf->is_full()) {
            split_leaf(leaf);
        }
    }

    bool find(const Key& key, Value& result) const {
        LeafNode* leaf = find_leaf(key);
        if (!leaf) return false;

        auto it = lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
        if (it != leaf->keys.end() && *it == key) {
            int index = it - leaf->keys.begin();
            result = leaf->values[index];
            return true;
        }
        return false;
    }

    void print() const {
        if (!root) {
            cout << "Empty tree" << endl;
            return;
        }
        print_node(root, 0);
    }

private:
    void print_node(Node* node, int depth) const {
        string indent(depth * 2, ' ');

        if (node->is_leaf) {
            LeafNode* leaf = static_cast<LeafNode*>(node);
            cout << indent << "hoja: ";
            for (size_t i = 0; i < leaf->keys.size(); ++i) {
                cout << leaf->keys[i] << "(" << leaf->values[i] << ") ";
            }
            cout << endl;
        } else {
            InternalNode* internal = static_cast<InternalNode*>(node);
            cout << indent << "interno: ";
            for (const auto& key : internal->keys) {
                cout << key << " ";
            }
            cout << endl;

            for (auto* child : internal->children) {
                print_node(child, depth + 1);
            }
        }
    }
};



int main() {
    BPlusTree<int, string> tree;
    
    //*probamos los inserts

    tree.insert(5, "sapo5");
    tree.insert(3, "perro3");
    tree.insert(7, "pato7");
    tree.insert(1, "gato1");
    tree.insert(9, "conejo9");

    tree.print();
    
    //*probamos la busqueda

    string value;
    if (tree.find(1, value)) {
        cout << "valor encontrado para la clave 1: " << value << endl;
    }
    

    return 0;
}