#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_NAME_LENGTH 256
#define MAX_PATH_LENGTH 512
#define MAX_CHILDREN 10

// Structure for a file system node (directory or file)
typedef struct Node {
    char name[MAX_NAME_LENGTH];
    bool is_directory;
    struct Node* parent;
    struct Node* children[MAX_CHILDREN];
    int num_children;
    // For files, you might want to store content or metadata
    // For simplicity, we'll just track the name and type for this example.
} Node;

// Global root of the file system tree
Node* root = NULL;

// Function to create a new node (directory or file)
Node* create_node(const char* name, bool is_dir) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strncpy(new_node->name, name, MAX_NAME_LENGTH - 1);
    new_node->name[MAX_NAME_LENGTH - 1] = '\0';
    new_node->is_directory = is_dir;
    new_node->parent = NULL;
    new_node->num_children = 0;
    for (int i = 0; i < MAX_CHILDREN; i++) {
        new_node->children[i] = NULL;
    }
    return new_node;
}

// Function to add a child node to a parent directory
bool add_child(Node* parent, Node* child) {
    if (!parent || !child || !parent->is_directory || parent->num_children >= MAX_CHILDREN) {
        return false;
    }
    child->parent = parent;
    parent->children[parent->num_children++] = child;
    return true;
}

// Function to get a node by its name within a directory
Node* get_child(Node* parent, const char* name) {
    if (!parent || !parent->is_directory) {
        return NULL;
    }
    for (int i = 0; i < parent->num_children; i++) {
        if (strcmp(parent->children[i]->name, name) == 0) {
            return parent->children[i];
        }
    }
    return NULL;
}

// Function to get a node by its absolute path
Node* get_node_by_path(const char* path) {
    if (!root) {
        return NULL;
    }
    if (strcmp(path, "/") == 0) {
        return root;
    }

    char temp_path[MAX_PATH_LENGTH];
    strncpy(temp_path, path, MAX_PATH_LENGTH - 1);
    temp_path[MAX_PATH_LENGTH - 1] = '\0';

    char* token;
    char* rest = temp_path;
    Node* current = root;

    // Skip the leading '/'
    if (rest[0] == '/') {
        rest++;
    }

    while ((token = strtok_r(rest, "/", &rest)) != NULL) {
        Node* next = get_child(current, token);
        if (!next) {
            return NULL; // Path not found
        }
        current = next;
    }
    return current;
}

// Function to create a directory
bool create_directory(const char* path) {
    if (!root) {
        fprintf(stderr, "Error: File system not initialized.\n");
        return false;
    }

    char temp_path[MAX_PATH_LENGTH];
    strncpy(temp_path, path, MAX_PATH_LENGTH - 1);
    temp_path[MAX_PATH_LENGTH - 1] = '\0';

    char* token;
    char* rest = temp_path;
    Node* current = root;
    char* parent_path_end = NULL;
    char* last_component = NULL;

    // Skip the leading '/'
    if (rest[0] == '/') {
        rest++;
        parent_path_end = temp_path + 1;
    } else {
        parent_path_end = temp_path;
    }

    while ((token = strtok_r(rest, "/", &rest)) != NULL) {
        Node* next = get_child(current, token);
        if (!next) {
            // If it's the last component, create the directory
            if (rest == NULL) {
                Node* new_dir = create_node(token, true);
                if (add_child(current, new_dir)) {
                    return true;
                } else {
                    free(new_dir);
                    fprintf(stderr, "Error: Could not add directory '%s' to '%s'.\n", token, current->name);
                    return false;
                }
            }
            return false; // Intermediate directory doesn't exist
        }
        current = next;
        if (rest != NULL) {
            parent_path_end += strlen(token) + 1; // +1 for the '/'
        }
        last_component = token;
    }

    fprintf(stderr, "Error: Directory '%s' already exists.\n", path);
    return false; // Path already exists
}

// Function to create a file
bool create_file(const char* path) {
    if (!root) {
        fprintf(stderr, "Error: File system not initialized.\n");
        return false;
    }

    char temp_path[MAX_PATH_LENGTH];
    strncpy(temp_path, path, MAX_PATH_LENGTH - 1);
    temp_path[MAX_PATH_LENGTH - 1] = '\0';

    char* last_slash = strrchr(temp_path, '/');
    if (!last_slash || last_slash == temp_path) {
        fprintf(stderr, "Error: Invalid file path '%s'. Must specify a parent directory.\n", path);
        return false;
    }

    *last_slash = '\0'; // Terminate the parent path
    char* filename = last_slash + 1;
    char* parent_path = temp_path;

    Node* parent_dir = get_node_by_path(parent_path);
    if (!parent_dir || !parent_dir->is_directory) {
        fprintf(stderr, "Error: Parent directory '%s' not found.\n", parent_path);
        return false;
    }

    if (get_child(parent_dir, filename)) {
        fprintf(stderr, "Error: File or directory '%s' already exists in '%s'.\n", filename, parent_path);
        return false;
    }

    Node* new_file = create_node(filename, false);
    if (add_child(parent_dir, new_file)) {
        return true;
    } else {
        free(new_file);
        fprintf(stderr, "Error: Could not add file '%s' to '%s'.\n", filename, parent_path);
        return false;
    }
}

// Function to list the contents of a directory
void list_directory(const char* path) {
    Node* dir_node = get_node_by_path(path);
    if (!dir_node || !dir_node->is_directory) {
        fprintf(stderr, "Error: Directory '%s' not found.\n", path);
        return;
    }
    printf("Contents of '%s':\n", path);
    for (int i = 0; i < dir_node->num_children; i++) {
        printf("%s%s\n", dir_node->children[i]->name, dir_node->children[i]->is_directory ? "/" : "");
    }
}

// Function to print the file system tree (for debugging)
void print_tree(Node* current, int depth) {
    if (current) {
        for (int i = 0; i < depth; i++) {
            printf("  ");
        }
        printf("%s%s\n", current->name, current->is_directory ? "/" : "");
        for (int i = 0; i < current->num_children; i++) {
            print_tree(current->children[i], depth + 1);
        }
    }
}

// Function to free the entire file system tree
void free_tree(Node* current) {
    if (current) {
        for (int i = 0; i < current->num_children; i++) {
            free_tree(current->children[i]);
        }
        free(current);
    }
}

int main() {
    // Initialize the root directory
    root = create_node("root", true);

    // Simulate user interactions based on the university use case

    // Create top-level directories
    create_directory("/academics");
    create_directory("/research");
    create_directory("/admin");
    create_directory("/users");
    create_directory("/shared");

    // Create sub-directories under /academics
    create_directory("/academics/courses");
    create_directory("/academics/student_records");
    create_directory("/academics/faculty_resources");
    create_directory("/academics/courses/CS");
    create_directory("/academics/courses/EE");
    create_directory("/academics/courses/CS/CS101");
    create_directory("/academics/courses/CS/CS201");

    // Create a file under a course directory
    create_file("/academics/courses/CS/CS101/syllabus_fall2024.pdf");
    create_file("/academics/courses/CS/CS201/lectures_week1.pptx");

    // Create user directories
    create_directory("/users/student123");
    create_directory("/users/prof_smith");

    // Create a file in a user directory
    create_file("/users/student123/assignment1.docx");

    // List contents of directories
    list_directory("/");
    list_directory("/academics");
    list_directory("/academics/courses/CS/CS101");
    list_directory("/users/student123");

    // Try to create a directory that already exists
    create_directory("/academics/courses");

    // Try to create a file in a non-existent directory
    create_file("/nonexistent/file.txt");

    // Get a node by path
    Node* syllabus = get_node_by_path("/academics/courses/CS/CS101/syllabus_fall2024.pdf");
    if (syllabus) {
        printf("Found: %s (is directory: %s)\n", syllabus->name, syllabus->is_directory ? "true" : "false");
    } else {
        printf("File not found.\n");
    }

    Node* non_existent = get_node_by_path("/academics/nonexistent");
    if (!non_existent) {
        printf("Path '/academics/nonexistent' not found as expected.\n");
    }

    // Print the entire tree structure
    printf("\nFile System Tree:\n");
    print_tree(root, 0);

    // Clean up memory
    free_tree(root);
    root = NULL;

    return 0;
}



Explanation of the Code:

#include directives: Include necessary header files for standard input/output, memory allocation, string manipulation, and boolean type.
#define macros: Define constants for maximum name length, path length, and the maximum number of children a directory can have.
Node structure: Represents a node in the file system tree.
name: Stores the name of the file or directory.
is_directory: A boolean flag indicating if the node is a directory.
parent: A pointer to the parent node.
children: An array of pointers to child nodes.
num_children: Keeps track of the number of child nodes.
root: A global pointer to the root node of the file system tree.
create_node(): Allocates memory for a new Node and initializes its fields.
add_child(): Adds a child node to a parent directory. It checks if the parent is a directory and if there's space for more children.
get_child(): Searches for a child node with a specific name within a parent directory.
get_node_by_path(): Traverses the tree based on the provided absolute path and returns the corresponding node. It handles the root path and splits the path into components.
create_directory(): Creates a new directory at the specified path. It parses the path, finds the parent directory, and adds the new directory node. It handles cases where the parent doesn't exist or the directory already exists.
create_file(): Creates a new file at the specified path. Similar to create_directory(), it finds the parent directory and adds a new file node.
list_directory(): Prints the names of the immediate children (files and subdirectories) of a given directory path.
print_tree(): A recursive function to print the entire file system tree structure with indentation to show the hierarchy. This is useful for debugging.
free_tree(): A recursive function to free all the dynamically allocated memory for the file system tree to prevent memory leaks.
main():
Initializes the root directory.
Simulates the creation of directories and files based on the university use case described in the previous responses.
Demonstrates the list_directory() and get_node_by_path() functions.
Includes error handling examples (trying to create an existing directory, creating a file in a non-existent directory).
Prints the file system tree.
Frees the allocated memory.
How to Compile and Run:

Save: Save the code in a file named filesystem.c.
Compile: Open a terminal or command prompt and use a C compiler (like GCC) to compile the code:
Bash

gcc filesystem.c -o filesystem
Run: Execute the compiled program:
Bash

./filesystem
This will output the results of the simulated file system operations and the tree structure to the console.

Limitations of this Simple Implementation:

No actual file storage: This code only simulates the directory structure and doesn't handle the actual content of files.
Fixed maximum children: The MAX_CHILDREN limit restricts the number of direct children a directory can have.
No error handling for path lengths: The code assumes paths and names won't exceed the defined maximum lengths.
Basic functionality: It lacks features like deleting files/directories, renaming, permissions, etc.
In-memory: The file system exists only in the program's memory and is lost when the program terminates.
This code provides a basic foundation for understanding the tree-structured organization of a file system in C. For a more robust and functional file system, you would need to interact with the operating system's file storage mechanisms.
