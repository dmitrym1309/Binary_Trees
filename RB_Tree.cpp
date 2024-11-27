#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include "Train.hpp"


class RedBlackTree {
    enum Color { RED, BLACK };

    struct Node {
        int train_number;
        int pos;
        Color color;
        Node* parent;
        Node* left;
        Node* right;

        Node(int train_number, int pos) 
            : train_number(train_number), pos(pos), color(RED), parent(nullptr), left(nullptr), right(nullptr) {}
    };

    Node* root = nullptr;
    std::string filename = "Data.txt";
    std::string binary = "7_1.bin";
    void rotate_left(Node*& node) {
        Node* child = node->right;
        node->right = child->left;
        if (child->left != nullptr)
            child->left->parent = node;
        child->parent = node->parent;

        if (node->parent == nullptr)
            root = child;
        else if (node == node->parent->left)
            node->parent->left = child;
        else
            node->parent->right = child;

        child->left = node;
        node->parent = child;
    }

    void rotate_right(Node*& node) {
        Node* child = node->left;
        node->left = child->right;
        if (child->right != nullptr)
            child->right->parent = node;
        child->parent = node->parent;

        if (node->parent == nullptr)
            root = child;
        else if (node == node->parent->left)
            node->parent->left = child;
        else
            node->parent->right = child;

        child->right = node;
        node->parent = child;
    }

    void fix_violation_on_insert(Node*& node) {
        Node* parent = nullptr;
        Node* grandparent = nullptr;

        while (node != root && node->color == RED && node->parent->color == RED) {
            parent = node->parent;
            grandparent = parent->parent;

            if (parent == grandparent->left) {
                Node* uncle = grandparent->right;
                if (uncle != nullptr && uncle->color == RED) {
                    grandparent->color = RED;
                    parent->color = BLACK;
                    uncle->color = BLACK;
                    node = grandparent;
                } else {
                    if (node == parent->right) {
                        rotate_left(parent);
                        node = parent;
                        parent = node->parent;
                    }
                    rotate_right(grandparent);
                    std::swap(parent->color, grandparent->color);
                    node = parent;
                }
            } else {
                Node* uncle = grandparent->left;
                if (uncle != nullptr && uncle->color == RED) {
                    grandparent->color = RED;
                    parent->color = BLACK;
                    uncle->color = BLACK;
                    node = grandparent;
                } else {
                    if (node == parent->left) {
                        rotate_right(parent);
                        node = parent;
                        parent = node->parent;
                    }
                    rotate_left(grandparent);
                    std::swap(parent->color, grandparent->color);
                    node = parent;
                }
            }
        }
        root->color = BLACK;
    }

    Node* minimum(Node* node) const {
        while (node->left != nullptr) {
            node = node->left;
        }
        return node;
    }

    void transplant(Node*& u, Node*& v) {
        if (u->parent == nullptr)
            root = v;
        else if (u == u->parent->left)
            u->parent->left = v;
        else
            u->parent->right = v;

        if (v != nullptr)
            v->parent = u->parent;
    }

    void fix_violation_on_delete(Node*& x) {
        while (x != root && (x == nullptr || x->color == BLACK)) {
            if (x == x->parent->left) {
                Node* sibling = x->parent->right;
                if (sibling->color == RED) {
                    sibling->color = BLACK;
                    x->parent->color = RED;
                    rotate_left(x->parent);
                    sibling = x->parent->right;
                }

                if ((sibling->left == nullptr || sibling->left->color == BLACK) &&
                    (sibling->right == nullptr || sibling->right->color == BLACK)) {
                    sibling->color = RED;
                    x = x->parent;
                } else {
                    if (sibling->right == nullptr || sibling->right->color == BLACK) {
                        if (sibling->left != nullptr)
                            sibling->left->color = BLACK;
                        sibling->color = RED;
                        rotate_right(sibling);
                        sibling = x->parent->right;
                    }

                    sibling->color = x->parent->color;
                    x->parent->color = BLACK;
                    if (sibling->right != nullptr)
                        sibling->right->color = BLACK;
                    rotate_left(x->parent);
                    x = root;
                }
            } else {
                Node* sibling = x->parent->left;
                if (sibling->color == RED) {
                    sibling->color = BLACK;
                    x->parent->color = RED;
                    rotate_right(x->parent);
                    sibling = x->parent->left;
                }

                if ((sibling->right == nullptr || sibling->right->color == BLACK) &&
                    (sibling->left == nullptr || sibling->left->color == BLACK)) {
                    sibling->color = RED;
                    x = x->parent;
                } else {
                    if (sibling->left == nullptr || sibling->left->color == BLACK) {
                        if (sibling->right != nullptr)
                            sibling->right->color = BLACK;
                        sibling->color = RED;
                        rotate_left(sibling);
                        sibling = x->parent->left;
                    }

                    sibling->color = x->parent->color;
                    x->parent->color = BLACK;
                    if (sibling->left != nullptr)
                        sibling->left->color = BLACK;
                    rotate_right(x->parent);
                    x = root;
                }
            }
        }

        if (x != nullptr)
            x->color = BLACK;
    }


    void delete_node(Node*& z) {
        Node* y = z;
        Node* x = nullptr;
        Color y_original_color = y->color;

        if (z->left == nullptr) {
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == nullptr) {
            x = z->left;
            transplant(z, z->left);
        } else {
            y = minimum(z->right);
            y_original_color = y->color;
            x = y->right;

            if (y->parent == z) {
                if (x != nullptr)
                    x->parent = y;
            } else {
                transplant(y, y->right);
                y->right = z->right;
                if (y->right != nullptr)
                    y->right->parent = y;
            }

            transplant(z, y);
            y->left = z->left;
            if (y->left != nullptr)
                y->left->parent = y;
            y->color = z->color;
        }

        delete z;

        if (y_original_color == BLACK && x != nullptr)
            fix_violation_on_delete(x);
    }

    void generate_dot(Node* node, std::ofstream& dot_file) const {
        if (node == nullptr) return;

        dot_file << "    \"" << node->train_number << "\" [label=\"" << node->train_number 
                 << "\", color=" << (node->color == RED ? "red" : "black") << ", fontcolor=white, style=filled];\n";

        if (node->left) {
            dot_file << "    \"" << node->train_number << "\" -> \"" << node->left->train_number << "\";\n";
            generate_dot(node->left, dot_file);
        } else {
            dot_file << "    \"" << node->train_number << "L\" [shape=point];\n";
            dot_file << "    \"" << node->train_number << "\" -> \"" << node->train_number << "L\";\n";
        }

        if (node->right) {
            dot_file << "    \"" << node->train_number << "\" -> \"" << node->right->train_number << "\";\n";
            generate_dot(node->right, dot_file);
        } else {
            dot_file << "    \"" << node->train_number << "R\" [shape=point];\n";
            dot_file << "    \"" << node->train_number << "\" -> \"" << node->train_number << "R\";\n";
        }
    }
    void remove_tree_from_node(Node* node) {
    if (node != nullptr) {
        remove_tree_from_node(node->left);
        remove_tree_from_node(node->right);
        delete node;
        }
    }
    void generate_dot_file(const std::string& dot_filename){
        std::cout << "Tree visualization (DOT format):\n";
        std::ofstream dot_file(dot_filename);
        dot_file << "digraph RBTree {\n";
        dot_file << "    node [shape=circle];\n";
        generate_dot(root, dot_file);
        dot_file << "}\n";
        dot_file.close();
        std::cout << "DOT file generated as " << dot_filename << std::endl;
    }
public:
    RedBlackTree(){
        convert_text_to_binary(filename, binary);
    };

    ~RedBlackTree() {
        remove_tree_from_node(root);
    }

    void insert(int train_number) {
        int pos = -1;
        Train* train = find_with_train_number(binary, train_number, pos);
        if (train == nullptr) {
            std::cout << "Train with number " << train_number << " not found in file." << std::endl;
            return;
        }
        delete train;

        Node* node = new Node(train_number, pos);
        Node* parent = nullptr;
        Node* current = root;

        while (current != nullptr) {
            parent = current;
            if (node->train_number < current->train_number)
                current = current->left;
            else
                current = current->right;
        }

        node->parent = parent;
        if (parent == nullptr)
            root = node;
        else if (node->train_number < parent->train_number)
            parent->left = node;
        else
            parent->right = node;

        fix_violation_on_insert(node);
    }

    void remove(int train_number) {
        Node* node = root;
        while (node != nullptr) {
            if (train_number == node->train_number) {
                delete_node(node);
                std::cout << "Node with train number " << train_number << " removed from tree.\n";
                return;
            } else if (train_number < node->train_number) {
                node = node->left;
            } else {
                node = node->right;
            }
        }
        std::cout << "Node with train number " << train_number << " not found in tree.\n";
    }
    Train find(int train_number) {
        Node* current = root;
        while (current != nullptr) {
            if (train_number < current->train_number) {
                current = current->left;
            } else if (train_number > current->train_number) {
                current = current->right;
            } else {
                Train* train = find_with_pos(binary, current->pos);
                if (train != nullptr) {
                    Train result = *train;
                    delete train;
                    return result;
                }
                return {};
            }
        }
        return {};
    }

    void generate_png(const std::string& dot_filename, const std::string& png_filename) {
        generate_dot_file(dot_filename);

        std::string command = "dot -Tpng " + dot_filename + " -o " + png_filename;
        int result = std::system(command.c_str());
        if (result == 0) {
            std::cout << "PNG file generated: " << png_filename << std::endl;
        } else {
            std::cerr << "Error generating PNG file. Ensure Graphviz is installed and accessible." << std::endl;
        }
    }
};

int main() {
    RedBlackTree tree;
    tree.insert(202);
    tree.insert(505);
    tree.insert(303);
    tree.insert(101);
    tree.insert(404);
    tree.insert(707);
    tree.insert(606);
    tree.generate_png("RB1.dot", "RB1.png");
    tree.remove(303);
    tree.generate_png("RB2.dot", "RB2.png");
}
