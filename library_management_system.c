
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_TITLE_LENGTH 100
#define MAX_AUTHOR_LENGTH 50
#define MAX_ISBN_LENGTH 20
#define MAX_NAME_LENGTH 50
#define MAX_ID_LENGTH 20
#define MAX_BOOKS 1000
#define MAX_USERS 500
#define MAX_STACK_SIZE 100
#define MAX_QUEUE_SIZE 100
#define MAX_SECTIONS 10
#define MAX_SHELVES 20
#define SAVE_FILE "library_data.dat"

/********************************************/
/* Data Structures and Type Definitions     */
/********************************************/

// Enum for Book Status
typedef enum {
    AVAILABLE,
    BORROWED,
    RESERVED
} BookStatus;

// Enum for User Status
typedef enum {
    ACTIVE,
    SUSPENDED,
    EXPIRED
} UserStatus;

// Book Structure
typedef struct Book {
    int id;
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    char isbn[MAX_ISBN_LENGTH];
    BookStatus status;
    struct Book* left;
    struct Book* right;
} Book;

// User Structure
typedef struct User {
    int id;
    char name[MAX_NAME_LENGTH];
    char user_id[MAX_ID_LENGTH];
    int age;
    char gender;
    UserStatus status;
    Book *borrowed;
    struct User* next;
} User;

// Book Queue Node for reservations
typedef struct QueueNode {
    int userId;
    struct QueueNode* next;
} QueueNode;

// Book Queue Structure
typedef struct {
    QueueNode* front;
    QueueNode* rear;
    int size;
} BookQueue;

// Browsing History Stack Node
typedef struct StackNode {
    int bookId;
    int userId;
    time_t returnDate;
    struct StackNode* next;
} StackNode;

typedef struct returnstack{
    int size ;
    StackNode *top;
}rstack;

// Browsing History Stack
typedef struct {
    StackNode* top;
    int size;
} BrowsingStack;

// Library Section Structure for N-ary Tree
typedef struct Section {
    char name[MAX_TITLE_LENGTH];
    struct Section* firstChild;
    struct Section* nextSibling;
} Section;

// Borrow Record
typedef struct BorrowRecord {
    int userId;
    int bookId;
    time_t borrowDate;
    time_t dueDate;
    bool returned;
    time_t returnDate;
    struct BorrowRecord* next;
} BorrowRecord;

// Global Variables
Book* bookRoot = NULL;
User* userList = NULL;
BookQueue bookQueues[MAX_BOOKS];
BrowsingStack browsingHistory;
Section* libraryRoot = NULL;
BorrowRecord* borrowRecords = NULL;
rstack* returnStack;
int numbooks = 0;
int numofuser = 0;
int borrowCount = 0;


/********************************************/
/*     Function prototypes (needed )        */
/********************************************/
void pushToReturnHistory(int bookId , int userId);
void pushToBrowsingHistory(int bookId);
/********************************************/
/* BOOK MANAGEMENT set of Functions          */
/********************************************/

// Create a new book
Book* addBook(int id, const char* title, const char* author, const char* isbn) {
    Book* newBook = (Book*)malloc(sizeof(Book));
    
    if (newBook == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    
    newBook->id = id;
    strncpy(newBook->title, title, MAX_TITLE_LENGTH - 1);
    newBook->title[MAX_TITLE_LENGTH - 1] = '\0';
    
    strncpy(newBook->author, author, MAX_AUTHOR_LENGTH - 1);
    newBook->author[MAX_AUTHOR_LENGTH - 1] = '\0';
    
    strncpy(newBook->isbn, isbn, MAX_ISBN_LENGTH - 1);
    newBook->isbn[MAX_ISBN_LENGTH - 1] = '\0';
    
    newBook->status = AVAILABLE;
    newBook->left = NULL;
    newBook->right = NULL;
    
    return newBook;
}

// Insert book into BST
Book* insertBook(Book* root, Book* newBook) {
    if (root == NULL) {
        return newBook;
    }
    
    if (newBook->id < root->id) {
        root->left = insertBook(root->left, newBook);
    } else if (newBook->id > root->id) {
        root->right = insertBook(root->right, newBook);
    } else {
        printf("Book with ID %d already exists!\n", newBook->id);
        free(newBook);
    }
    
    return root;
}

// Search for book by ID
Book* searchBookById(Book* root, int id) {
    if (root == NULL || root->id == id) {
        return root;
    }
    
    if (id < root->id) {
        return searchBookById(root->left, id);
    }
    
    return searchBookById(root->right, id);
}

// Search for book by title (partial match)
Book* searchBookByTitle(Book* root, const char* title) {
    if (root == NULL) {
        return NULL;
    }
    
    if (strstr(root->title, title) != NULL) {
        printf("Found: %s by %s (ID = %d)\n", root->title, root->author, root->id);
        return root;
    }
    
    Book* leftResult = searchBookByTitle(root->left, title);
    if (leftResult != NULL) {
        return leftResult;
    }
    
    return searchBookByTitle(root->right, title);
}
// Find minimum value node in BST (helper for deletion)

Book* findMinValueNode(Book* root) {
    Book* current = root;
    
    while (current && current->left != NULL) {
        current = current->left;
    }
    
    return current;
}

// Delete book from BST
Book* deleteBook(Book* root, int id) {
    if (root == NULL) {
        return root;
    }
    
    if (id < root->id) {
        root->left = deleteBook(root->left, id);
    } else if (id > root->id) {
        root->right = deleteBook(root->right, id);
    } else {
        // Case 1: Leaf Node
        if (root->left == NULL && root->right == NULL) {
            free(root);
            return NULL;
        }
        // Case 2: One child
        else if (root->left == NULL) {
            Book* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            Book* temp = root->left;
            free(root);
            return temp;
        }
        
        // Case 3: Two children
        Book* temp = findMinValueNode(root->right);
        root->id = temp->id;
        strcpy(root->title, temp->title);
        strcpy(root->author, temp->author);
        strcpy(root->isbn, temp->isbn);
        root->status = temp->status;
        
        root->right = deleteBook(root->right, temp->id);
    }
    
    return root;
}

// Function to display a book's details
void displayBook(Book* book) {
    if (book != NULL) {
        printf("---------------------------\n");
        printf("ID: %d\n", book->id);
        printf("Title: %s\n", book->title);
        printf("Author: %s\n", book->author);
        printf("ISBN: %s\n", book->isbn);
        
        printf("Status: ");
        switch (book->status) {
            case AVAILABLE:
                printf("Available\n");
                break;
            case BORROWED:
                printf("Borrowed\n");
                break;
            case RESERVED:
                printf("Reserved\n");
                break;
        }
        printf("---------------------------\n");
    }
}

// In-order traversal to display all books
void displayAllBooks(Book* root) {
    if (root != NULL) {
        displayAllBooks(root->left);
        displayBook(root);
        displayAllBooks(root->right);
    }
}

// Add a new book via user input
void bookInSys() {
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    char isbn[MAX_ISBN_LENGTH];
    while ((getchar()) != '\n');

    printf("Enter book title: ");
    fgets(title, MAX_TITLE_LENGTH, stdin);
    title[strcspn(title, "\n")] = '\0';  // Remove newline


    printf("Enter author name: ");
    fgets(author, MAX_AUTHOR_LENGTH, stdin);
    author[strcspn(author, "\n")] = '\0';  // Remove newline
    
    printf("Enter ISBN: ");
    fgets(isbn, MAX_ISBN_LENGTH, stdin);
    isbn[strcspn(isbn, "\n")] = '\0';  // Remove newline
    
    numbooks++;
    Book* newBook = addBook(numbooks, title, author, isbn);
    bookRoot = insertBook(bookRoot, newBook);
    
    printf("Book added successfully!\n");
}

// Edit book details
void editBook() {
    int id;
    printf("Enter book ID to edit: ");
    scanf("%d", &id);
    
    Book* book = searchBookById(bookRoot, id);
    if (book == NULL) {
        printf("Book not found!\n");
        return;
    }
    
    printf("Current details:\n");
    displayBook(book);
    
    
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    char isbn[MAX_ISBN_LENGTH];
    int choice;
    do{
    printf("\nEnter new details (or leave blank to keep current):\n");
    printf("1. Edit title\n");
    printf("2. Edit author\n");
    printf("3. Edit ISBN\n");
    printf("4. Back\n");
    printf("Enter choice: ");
    scanf("%d", &choice);
    printf("----------------------------------------------------\n");
    while ((getchar()) != '\n');
    switch (choice) {
        case 1:
            printf("Enter new title: ");
            fgets(title, MAX_TITLE_LENGTH, stdin);
            title[strcspn(title, "\n")] = '\0';
            if (strlen(title) > 0) {
                strncpy(book->title, title, MAX_TITLE_LENGTH - 1);
                book->title[MAX_TITLE_LENGTH - 1] = '\0';
            }
            break;
        case 2:
            printf("Enter new author: ");
            fgets(author, MAX_AUTHOR_LENGTH, stdin);
            author[strcspn(author, "\n")] = '\0';
            if (strlen(author) > 0) {
                strncpy(book->author, author, MAX_AUTHOR_LENGTH - 1);
                book->author[MAX_AUTHOR_LENGTH - 1] = '\0';
            }
            break;
        case 3:
            printf("Enter new ISBN: ");
            fgets(isbn, MAX_ISBN_LENGTH, stdin);
            isbn[strcspn(isbn, "\n")] = '\0';
            if (strlen(isbn) > 0) {
                strncpy(book->isbn, isbn, MAX_ISBN_LENGTH - 1);
                book->isbn[MAX_ISBN_LENGTH - 1] = '\0';
            }
            break;
        case 4:
            return;
        default:
            printf("Invalid choice!\n");
    }
}while (choice !=4);
    printf("Book updated successfully!\n");
}

// View book details
void viewBook() {
    int id;
    printf("Enter book ID to view: ");
    scanf("%d", &id);
    
    Book* book = searchBookById(bookRoot, id);
    if (book == NULL) {
        printf("Book not found!\n");
        return;
    }
    
    displayBook(book);
}

// Delete a book
void deleteBookMenu() {
    int id;
    printf("Enter book ID to delete: ");
    scanf("%d", &id);
    
    Book* book = searchBookById(bookRoot, id);
    if (book == NULL) {
        printf("Book not found!\n");
        return;
    }
    
    // Check if the book is borrowed
    if (book->status != AVAILABLE) {
        printf("Cannot delete book as it is currently borrowed or reserved!\n");
        return;
    }
    
    bookRoot = deleteBook(bookRoot, id);
    printf("Book deleted successfully!\n");
}

// Book management menu
void manageBooks() {
    int choice;
    
    do {
        printf("\n=== Book Management ===\n");
        printf("1. Add Book\n");
        printf("2. Edit Book\n");
        printf("3. View Book\n");
        printf("4. Delete Book\n");
        printf("5. List All Books\n");
        printf("6. Back\n");
        printf("Choose an option: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                bookInSys();
                break;
            case 2:
                editBook();
                break;
            case 3:
                viewBook();
                break;
            case 4:
                deleteBookMenu();
                break;
            case 5:
                displayAllBooks(bookRoot);
                break;
            case 6:
                return;
            default:
                printf("Invalid choice!\n");
        }
    } while (choice != 6);
}

/********************************************/
/* User Management Functions                */
/********************************************/

// Create a new user
User* createUser(int id, const char* name, const char* user_id, int age, char gender) {
    User* newUser = (User*)malloc(sizeof(User));
    if (newUser == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    
    newUser->id = id;
    strncpy(newUser->name, name, MAX_NAME_LENGTH - 1);
    newUser->name[MAX_NAME_LENGTH - 1] = '\0';
    
    strncpy(newUser->user_id, user_id, MAX_ID_LENGTH - 1);
    newUser->user_id[MAX_ID_LENGTH - 1] = '\0';
    
    newUser->age = age;
    newUser->gender = gender;
    newUser->status = ACTIVE;
    newUser->next = NULL;
    
    return newUser;
}

// Add user to linked list
void addUserToList(User* newUser) {
    if (userList == NULL) {
        userList = newUser;
    } else {
        User* current = userList;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newUser;
    }
}

// Search for user by ID
User* searchUserById(int id) {
    User* current = userList;
    while (current != NULL) {
        if (current->id == id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Search for user by name
User* searchUserByName(const char* name) {
    User* current = userList;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Display user details
void displayUser(User* user) {
    printf("---------------------------\n");
    if (user != NULL) {
        printf("ID: %d\n", user->id);
        printf("Name: %s\n", user->name);
        printf("User ID: %s\n", user->user_id);
        printf("Age: %d\n", user->age);
        printf("Gender: %c\n", user->gender);
        
        printf("Status: ");
        switch (user->status) {
            case ACTIVE:
                printf("Active\n");
                break;
            case SUSPENDED:
                printf("Suspended\n");
                break;
            case EXPIRED:
                printf("Expired\n");
                break;
        }
        printf("---------------------------\n");
    }
}

// Display all users
void displayAllUsers() {
    User* current = userList;
    if (current == NULL) {
        printf("No users found!\n");
        return;
    }
    
    printf("\n=== All Users ===\n");
    while (current != NULL) {
        displayUser(current);
        current = current->next;
    }
}

// Delete user
void delusernode(int id) {
    if (userList == NULL) {
        printf("No users in the system!\n");
        return;
    }
    
    // Handle case where head is to be deleted
    if (userList->id == id) {
        User* temp = userList;
        userList = userList->next;
        free(temp);
        printf("User deleted successfully!\n");
        return;
    }
    
    // Find user to delete
    User* current = userList;
    User* prev = NULL;
    
    while (current != NULL && current->id != id) {
        prev = current;
        current = current->next;
    }
    
    if (current == NULL) {
        printf("User not found!\n");
        return;
    }
    
    // Delete the node
    prev->next = current->next;
    free(current);
    printf("User deleted successfully!\n");
}

// Add a new user via user input
void userInSys() {
    char name[MAX_NAME_LENGTH];
    char user_id[MAX_ID_LENGTH];
    int age;
    char gender;
    while ((getchar()) != '\n');

    printf("Enter user name: ");
    fgets(name, MAX_NAME_LENGTH, stdin);
    name[strcspn(name, "\n")] = '\0';  // Remove newline
    
    printf("Enter user ID: ");
    fgets(user_id, MAX_ID_LENGTH, stdin);
    user_id[strcspn(user_id, "\n")] = '\0';  // Remove newline
    
    printf("Enter age: ");
    scanf("%d", &age);
    
    printf("Enter gender (M/F): ");
    scanf(" %c", &gender);
    
    numofuser++;
    User* newUser = createUser(numofuser, name, user_id, age, gender);
    addUserToList(newUser);
    
    printf("User added successfully!\n");
}

// Edit user details
void editUser() {
    int id;
    printf("Enter user ID to edit: ");
    scanf("%d", &id);
    
    User* user = searchUserById(id);
    if (user == NULL) {
        printf("User not found!\n");
        return;
    }
    
    printf("Current details:\n");
    displayUser(user);
    
    int choice;
    char name[MAX_NAME_LENGTH];
    char user_id[MAX_ID_LENGTH];
    int age;
    char gender;

    
    do{
    printf("\nEnter new details (or leave blank to keep current):\n");
    printf("1. Edit name\n");
    printf("2. Edit user ID\n");
    printf("3. Edit age\n");
    printf("4. Edit gender\n");
    printf("5. Edit status\n");
    printf("6. Back\n");
    printf("Enter choice: ");
    printf("----------------------------------------------------\n");
    scanf("%d", &choice);

    while ((getchar()) != '\n'); 
    
    switch (choice) {
        case 1:
            printf("Enter new name: ");
            fgets(name, MAX_NAME_LENGTH, stdin);
            name[strcspn(name, "\n")] = '\0';
            if (strlen(name) > 0) {
                strncpy(user->name, name, MAX_NAME_LENGTH - 1);
                user->name[MAX_NAME_LENGTH - 1] = '\0';
            }
            break;
        case 2:
            printf("Enter new user ID: ");
            fgets(user_id, MAX_ID_LENGTH, stdin);
            user_id[strcspn(user_id, "\n")] = '\0';
            if (strlen(user_id) > 0) {
                strncpy(user->user_id, user_id, MAX_ID_LENGTH - 1);
                user->user_id[MAX_ID_LENGTH - 1] = '\0';
            }
            break;
        case 3:
            printf("Enter new age: ");
            scanf("%d", &age);
            user->age = age;
            break;
        case 4:
            printf("Enter new gender (M/F): ");
            scanf(" %c", &gender);
            user->gender = gender;
            break;
        case 5:
            printf("Select new status:\n");
            printf("1. Active\n");
            printf("2. Suspended\n");
            printf("3. Expired\n");
            printf("Enter choice: ");
            scanf("%d", &choice);
            
            switch (choice) {
                case 1:
                    user->status = ACTIVE;
                    break;
                case 2:
                    user->status = SUSPENDED;
                    break;
                case 3:
                    user->status = EXPIRED;
                    break;
                default:
                    printf("Invalid choice!\n");
            }
            break;
        case 6:
            return;
        default:
            printf("Invalid choice!\n");
    }
}while(choice != 6 );
    printf("User updated successfully!\n");
}

// View user details
void viewUser() {
    int id;
    printf("Enter user ID to view: ");
    scanf("%d", &id);
    
    User* user = searchUserById(id);
    if (user == NULL) {
        printf("User not found!\n");
        return;
    }
    
    displayUser(user);
}

// Delete user menu function
void deleteUser() {
    int id;
    printf("Enter user ID to delete: ");
    scanf("%d", &id);
    
    User* user = searchUserById(id);
    if (user == NULL) {
        printf("User not found!\n");
        return;
    }
    
    // Check if user has borrowed books
    BorrowRecord* current = borrowRecords;
    while (current != NULL) {
        if (current->userId == id && !current->returned) {
            printf("Cannot delete user as they have borrowed books!\n");
            return;
        }
        current = current->next;
    }
    
    delusernode(id);
}

// User management menu
void manageUsers() {
    int choice;
    
    do {
        printf("\n=== User Management ===\n");
        printf("1. Add User\n");
        printf("2. Edit User\n");
        printf("3. View User\n");
        printf("4. Delete User\n");
        printf("5. List All Users\n");
        printf("6. Back\n");
        printf("Choose an option: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                userInSys();
                break;
            case 2:
                editUser();
                break;
            case 3:
                viewUser();
                break;
            case 4:
                deleteUser();
                break;
            case 5:
                displayAllUsers();
                break;
            case 6:
                return;
            default:
                printf("Invalid choice!\n");
        }
    } while (choice != 6);
}

/********************************************/
/* Book Queue Functions                     */
/********************************************/

// Initialize book queue
void initializeBookQueue(int bookId) {
    bookQueues[bookId].front = NULL;
    bookQueues[bookId].rear = NULL;
    bookQueues[bookId].size = 0;
}

// Check if queue is empty
int isQueueEmpty(int bookId) {
    return bookQueues[bookId].front == NULL;
}

// Enqueue a user for a book
void enqueueUser(int bookId, int userId) {
    QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
    if (newNode == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }
    
    newNode->userId = userId;
    newNode->next = NULL;
    
    // If queue is empty
    if (bookQueues[bookId].rear == NULL) {
        bookQueues[bookId].front = newNode;
        bookQueues[bookId].rear = newNode;
    } else {
        bookQueues[bookId].rear->next = newNode;
        bookQueues[bookId].rear = newNode;
    }
    
    bookQueues[bookId].size++;
}

// Dequeue a user from book queue
int dequeueUser(int bookId) {
    if (isQueueEmpty(bookId)) {
        printf("No users in queue for this book!\n");
        return -1;
    }
    
    QueueNode* temp = bookQueues[bookId].front;
    int userId = temp->userId;
    
    bookQueues[bookId].front = bookQueues[bookId].front->next;
    
    // If front becomes NULL, rear should also be NULL
    if (bookQueues[bookId].front == NULL) {
        bookQueues[bookId].rear = NULL;
    }
    
    free(temp);
    bookQueues[bookId].size--;
    
    return userId;
}

// Display all users in queue for a book
void displayBookQueue(int bookId) {
    if (isQueueEmpty(bookId)) {
        printf("No users in queue for this book!\n");
        return;
    }
    
    QueueNode* current = bookQueues[bookId].front;
    int position = 1;
    
    printf("Users in queue for book ID %d:\n", bookId);
    printf("---------------------------\n");
    
    while (current != NULL) {
        User* user = searchUserById(current->userId);
        printf("Position %d: %s (ID: %d)\n", position, user->name, user->id);
        current = current->next;
        position++;
    }
}

/********************************************/
/* Borrow Record Functions                  */
/********************************************/

// Create a new borrow record
BorrowRecord* createBorrowRecord(int userId, int bookId) {
    BorrowRecord* newRecord = (BorrowRecord*)malloc(sizeof(BorrowRecord));
    if (newRecord == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    
    newRecord->userId = userId;
    newRecord->bookId = bookId;
    newRecord->borrowDate = time(NULL);
    newRecord->dueDate = newRecord->borrowDate + (1209600); // 14 days loan period
    newRecord->returned = false;
    newRecord->returnDate = 0;
    newRecord->next = NULL;
    
    return newRecord;
}

// Add borrow record to list
void addBorrowRecord(BorrowRecord* newRecord) {
    if (borrowRecords == NULL) {
        borrowRecords = newRecord;
    } else {
        BorrowRecord* current = borrowRecords;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newRecord;
    }
    borrowCount++;
}

// Mark a book as returned
void returnBook(int bookId, int userId) {
    BorrowRecord* current = borrowRecords;
    BorrowRecord* lastReturn = NULL;
    
    while (current != NULL) {
        if (current->bookId == bookId && current->userId == userId && !current->returned) {
            current->returned = true;
            current->returnDate = time(NULL);
            lastReturn = current;
            
            // Update book status
            Book* book = searchBookById(bookRoot, bookId);
            if (book != NULL) {
                // Check if there are users in queue
                if (!isQueueEmpty(bookId)) {
                    book->status = RESERVED;
                } else {
                    book->status = AVAILABLE;
                }
            }
            pushToReturnHistory(bookId,userId);
            printf("Book returned successfully!\n");
            return;
        }
        current = current->next;
    }
    
    printf("No matching borrow record found!\n");
}

void pushToReturnHistory(int bookId , int userId) {
    StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));
    newNode->next = NULL ; 
    if (newNode == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }
    
    newNode->bookId = bookId;
    newNode->userId = userId;
    newNode->returnDate = time(NULL);
    newNode->next = returnStack->top;
    returnStack->top = newNode;
    
    // Limit stack size
    if (returnStack->size > MAX_STACK_SIZE) {
        // Remove bottom node
        StackNode* current = returnStack->top;
        StackNode* prev = NULL;
        
        while (current->next != NULL) {
            prev = current;
            current = current->next;
        }
        
        if (prev != NULL) {
            prev->next = NULL;
            free(current);
            returnStack->size--;
        }
    }
}

void displayLastReturn(){

    Book* returnedBook = searchBookById(bookRoot , returnStack->top->bookId);
    User* user = searchUserById(returnStack->top->userId);
    printf("%s has been returned by %s" , returnedBook->title , user->name);

}

void undoLastReturn(){} 
// Undo last return
void undoLastReturn2() {
    BorrowRecord* current = borrowRecords;
    BorrowRecord* lastReturn = NULL;
    time_t latestReturnTime = 0;
    
    // Find the most recently returned book
    while (current != NULL) {
        if (current->returned && current->returnDate > latestReturnTime) {
            lastReturn = current;
            latestReturnTime = current->returnDate;
        }
        current = current->next;
    }
    
    if (lastReturn != NULL) {
        // Check if the book is still available
        Book* book = searchBookById(bookRoot, lastReturn->bookId);
        if (book != NULL && book->status == AVAILABLE) {
            lastReturn->returned = false;
            lastReturn->returnDate = 0;
            book->status = BORROWED;
            
            printf("Last return undone successfully!\n");
            
            User* user = searchUserById(lastReturn->userId);
            if (user != NULL) {
                printf("Book '%s' is now borrowed again by %s\n", book->title, user->name);
            }
        } else {
            printf("Cannot undo return as book is no longer available!\n");
        }
    } else {
        printf("No recent returns found to undo!\n");
    }
}

// Display all borrow records
void displayAllBorrowRecords() {
    BorrowRecord* current = borrowRecords;
    if (current == NULL) {
        printf("No borrow records found!\n");
        return;
    }
    
    printf("\n=== All Borrow Records ===\n");
    printf("---------------------------\n");
    
    while (current != NULL) {
        Book* book = searchBookById(bookRoot, current->bookId);
        User* user = searchUserById(current->userId);
        
        if (book != NULL && user != NULL) {
            printf("Record ID: %d\n", borrowCount);
            printf("Book: %s\n", book->title);
            printf("User: %s\n", user->name);
            
            // Convert time_t to readable format
            char borrowDateStr[26];
            char dueDateStr[26];
            
            strftime(borrowDateStr, sizeof(borrowDateStr), "%Y-%m-%d %H:%M:%S", localtime(&current->borrowDate));
            strftime(dueDateStr, sizeof(dueDateStr), "%Y-%m-%d %H:%M:%S", localtime(&current->dueDate));
            
            printf("Borrow Date: %s\n", borrowDateStr);
            printf("Due Date: %s\n", dueDateStr);
            
            if (current->returned) {
                char returnDateStr[26];
                strftime(returnDateStr, sizeof(returnDateStr), "%Y-%m-%d %H:%M:%S", localtime(&current->returnDate));
                printf("Return Date: %s\n", returnDateStr);
                printf("Status: Returned\n");
            } else {
                printf("Status: Borrowed\n");
                
                // Check if overdue
                time_t now = time(NULL);
                if (now > current->dueDate) {
                    printf("OVERDUE!\n");
                    
                    // Calculate overdue days
                    int overdueDays = (now - current->dueDate) / (24 * 60 * 60);
                    printf("Overdue by: %d days\n", overdueDays);
                }
            }
        } else {
            printf("Invalid record (Book or User deleted)\n");
        }
        
        printf("---------------------------\n");
        current = current->next;
    }
}

// Borrow a book
void borrowBook() {

    char title[MAX_TITLE_LENGTH-1],name[MAX_NAME_LENGTH];

    
    printf("Enter Book Title (or part of it): ");
    while ((getchar()) != '\n');
    fgets(title, MAX_TITLE_LENGTH, stdin);
    title[strcspn(title, "\n")] = '\0';

    printf("Enter User's name: ");
    fgets(name, MAX_NAME_LENGTH, stdin);
    name[strcspn(name, "\n")] = '\0';

    // Validate book and user
    Book* book = searchBookByTitle(bookRoot, title);
    User* user = searchUserByName(name);
    
    if (book == NULL) {
        printf("Book not found!\n");
        return;
    }
    
    if (user == NULL) {
        printf("User not found!\n");
        return;
    }
    
    // Check if user is active
    if (user->status != ACTIVE) {
        printf("User account is not active!\n");
        return;
    }
    
    // Check if book is available
    if (book->status == AVAILABLE) {
        // Create borrow record
        BorrowRecord* newRecord = createBorrowRecord(user->id, book->id);
        addBorrowRecord(newRecord);
        
        // Update book status
        book->status = BORROWED;
        
        // Add to browsing history
        int bid = book->id;
        pushToBrowsingHistory(bid);
        
        printf("Book borrowed successfully!\n");
        
        // Format due date
        char dueDateStr[26];
        strftime(dueDateStr, sizeof(dueDateStr), "%Y-%m-%d %H:%M:%S", localtime(&newRecord->dueDate));
        printf("Due Date: %s\n", dueDateStr);
    } else if (book->status == BORROWED) {
        printf("Book is already borrowed. Would you like to join the reservation queue? (1-Yes/0-No): ");
        int choice;
        scanf("%d", &choice);
        
        if (choice == 1) {
            // Initialize queue if not initialized
            if (bookQueues[book->id].size == 0) {
                initializeBookQueue(book->id);
            }
            
            // Add user to queue
            enqueueUser(book->id, user->id);
            printf("User added to reservation queue!\n");
        }
    } else if (book->status == RESERVED) {
        printf("Book is reserved. Would you like to join the reservation queue? (1-Yes/0-No): ");
        int choice;
        scanf("%d", &choice);
        
        if (choice == 1) {
            // Add user to queue
            enqueueUser(book->id, user->id);
            printf("User added to reservation queue!\n");
        }
    }
}

// Reserve a book
void reserveBook() {
    int bookId, userId;
    
    printf("Enter Book ID to reserve: ");
    scanf("%d", &bookId);
    
    printf("Enter User ID: ");
    scanf("%d", &userId);
    
    // Validate book and user
    Book* book = searchBookById(bookRoot, bookId);
    User* user = searchUserById(userId);
    
    if (book == NULL) {
        printf("Book not found!\n");
        return;
    }
    
    if (user == NULL) {
        printf("User not found!\n");
        return;
    }
    
    // Check if user is active
    if (user->status != ACTIVE) {
        printf("User account is not active!\n");
        return;
    }
    
    // Initialize queue if not initialized
    if (bookQueues[bookId].size == 0) {
        initializeBookQueue(bookId);
    }

    //checks if is already in queue 
    int Uid;bool exist = false ;
    for(int i=0 ; i<bookQueues[bookId].size ; i++){
        Uid = dequeueUser(bookId);

        if(Uid == userId){
        exist = true;
        }

        enqueueUser(bookId , Uid);
    }
    if(exist){
        printf("the user is already in the reservation queue");
        return;
    }

    // Add user to queue
    enqueueUser(bookId, userId);
    
    // If book is available, mark as reserved
    if (book->status == AVAILABLE) {
        book->status = RESERVED;
        printf("Book reserved successfully! You can pick it up now.\n");
    } else {
        printf("User added to reservation queue!\n");
        printf("Your position in queue: %d\n", bookQueues[bookId].size);
    }
}

// Cancel reservation
void cancelReservation() {
    int bookId, userId;
    
    printf("Enter Book ID: ");
    scanf("%d", &bookId);
    
    printf("Enter User ID: ");
    scanf("%d", &userId);
    
    // Validate book
    Book* book = searchBookById(bookRoot, bookId);
    if (book == NULL) {
        printf("Book not found!\n");
        return;
    }
    
    // Check if queue exists
    if (bookQueues[bookId].size == 0) {
        printf("No reservations for this book!\n");
        return;
    }
    
    // Find and remove user from queue
    QueueNode* current = bookQueues[bookId].front;
    QueueNode* prev = NULL;
    
    while (current != NULL) {
        if (current->userId == userId) {
            // Remove from queue
            if (prev == NULL) {
                // User is at front of queue
                bookQueues[bookId].front = current->next;
                
                if (bookQueues[bookId].front == NULL) {
                    bookQueues[bookId].rear = NULL;
                }
            } else {
                prev->next = current->next;
                
                if (current == bookQueues[bookId].rear) {
                    bookQueues[bookId].rear = prev;
                }
            }
            
            free(current);
            bookQueues[bookId].size--;
            
            printf("Reservation cancelled successfully!\n");
            
            // If queue is empty and book is reserved, change status to available
            if (bookQueues[bookId].size == 0 && book->status == RESERVED) {
                book->status = AVAILABLE;
            }
            
            return;
        }
        
        prev = current;
        current = current->next;
    }
    
    printf("No reservation found for this user and book!\n");
}

// Check book availability
void checkBookAvailability() {
    int bookId;
    
    printf("Enter Book ID to check: ");
    scanf("%d", &bookId);
    
    Book* book = searchBookById(bookRoot, bookId);
    if (book == NULL) {
        printf("Book not found!\n");
        return;
    }
    
    printf("Book: %s by %s\n", book->title, book->author);
    printf("Status: ");
    
    switch (book->status) {
        case AVAILABLE:
            printf("Available for borrowing\n");
            break;
        case BORROWED:
            printf("Currently borrowed\n");
            
            // Find who borrowed it
            BorrowRecord* current = borrowRecords;
            while (current != NULL) {
                if (current->bookId == bookId && !current->returned) {
                    User* user = searchUserById(current->userId);
                    if (user != NULL) {
                        printf("Borrowed by: %s\n", user->name);
                        
                        // Format due date
                        char dueDateStr[26];
                        strftime(dueDateStr, sizeof(dueDateStr), "%Y-%m-%d %H:%M:%S", localtime(&current->dueDate));
                        printf("Due Date: %s\n", dueDateStr);
                    }
                    break;
                }
                current = current->next;
            }
            
            // Show queue length
            if (bookQueues[bookId].size > 0) {
                printf("Reservation Queue Length: %d\n", bookQueues[bookId].size);
            }
            break;
        case RESERVED:
            printf("Reserved\n");
            
            // Show first person in queue
            if (bookQueues[bookId].front != NULL) {
                User* user = searchUserById(bookQueues[bookId].front->userId);
                if (user != NULL) {
                    printf("Reserved for: %s\n", user->name);
                }
                
                // Show queue length
                if (bookQueues[bookId].size > 1) {
                    printf("Additional Reservations: %d\n", bookQueues[bookId].size - 1);
                }
            }
            break;
    }
}

// Process next reservation
void processNextReservation() {
    int bookId;
    
    printf("Enter Book ID: ");
    scanf("%d", &bookId);
    
    Book* book = searchBookById(bookRoot, bookId);
    if (book == NULL) {
        printf("Book not found!\n");
        return;
    }
    
    if (book->status != AVAILABLE && book->status != RESERVED) {
        printf("Book is not available for reservation processing!\n");
        return;
    }
    
    if (isQueueEmpty(bookId)) {
        printf("No reservations in queue for this book!\n");
        return;
    }
    
    // Dequeue next user
    int userId = dequeueUser(bookId);
    if (userId == -1) {
        return;
    }
    
    User* user = searchUserById(userId);
    if (user == NULL) {
        printf("User not found!\n");
        return;
    }
    
    // Create borrow record
    BorrowRecord* newRecord = createBorrowRecord(userId, bookId);
    addBorrowRecord(newRecord);
    
    // Update book status
    book->status = BORROWED;
    
    printf("Reservation processed successfully!\n");
    printf("Book '%s' is now borrowed by %s\n", book->title, user->name);
    
    // Format due date
    char dueDateStr[26];
    strftime(dueDateStr, sizeof(dueDateStr), "%Y-%m-%d %H:%M:%S", localtime(&newRecord->dueDate));
    printf("Due Date: %s\n", dueDateStr);
}

// Return a borrowed book
void returnBookMenu() {
    int bookId, userId;
    
    printf("Enter Book ID to return: ");
    scanf("%d", &bookId);
    
    printf("Enter User ID: ");
    scanf("%d", &userId);
    
    // Validate book and user
    Book* book = searchBookById(bookRoot, bookId);
    User* user = searchUserById(userId);
    
    if (book == NULL) {
        printf("Book not found!\n");
        return;
    }
    
    if (user == NULL) {
        printf("User not found!\n");
        return;
    }
    
    // Check if book is borrowed
    if (book->status != BORROWED) {
        printf("This book is not currently borrowed!\n");
        return;
    }
    
    // Verify the user has borrowed this book
    BorrowRecord* current = borrowRecords;
    bool userHasBorrowed = false;
    while (current != NULL) {
        if (current->bookId == bookId && current->userId == userId && !current->returned) {
            userHasBorrowed = true;
            break;
        }
        current = current->next;
    }
    
    if (!userHasBorrowed) {
        printf("This user has not borrowed this book!\n");
        return;
    }
    
    // Return the book
    returnBook(bookId, userId);
    
    // Check if there are users in queue
    if (!isQueueEmpty(bookId)) {
        printf("There are users waiting for this book. Process next reservation? (1-Yes/0-No): ");
        int choice;
        scanf("%d", &choice);
        
        if (choice == 1) {
            processNextReservation();
        }
    }
}

//displays all the books currently being borrowed by a used
void displayAllBorrowByUser(int userId){
    BorrowRecord* current = borrowRecords;
    User* user = searchUserById(userId);
    int count = 0;

    while (current != NULL){
        if(current->userId == userId);
        Book* book = searchBookById(bookRoot , current->bookId);
        printf("%s has borrowed %s \n " ,user->name , book->title );
        current = current->next;
        count++;
    }

    if(count == 0){
        printf("%s has no borrowed books for the time being" , user->name);
    }
    else{
        printf("%s has borrowed %d books" , user->name , count);
    }

    
}

// Book borrowing management menu
void manageBorrowing() {
    int choice;
    
    do {
        printf("\n=== Borrow Management ===\n");
        printf("1. Borrow Book\n");
        printf("2. Return Book\n");
        printf("3. Reserve Book\n");
        printf("4. Cancel Reservation\n");
        printf("5. Check Book Availability\n");
        printf("6. Display Book Queue\n");
        printf("7. Process Next Reservation\n");
        printf("8. View last return\n");
        printf("9. Undo Last Return\n");
        printf("10. View All Borrow Records \n");
        printf("11. View all the books borrowed by User \n");
        printf("12. Back\n");
        printf("Choose an option: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                borrowBook();
                break;
            case 2:
                returnBookMenu();
                break;
            case 3:
                reserveBook();
                break;
            case 4:
                cancelReservation();
                break;
            case 5:
                checkBookAvailability();
                break;
            case 6: {
                int bookId;
                printf("Enter Book ID to view queue: ");
                scanf("%d", &bookId);
                displayBookQueue(bookId);
                break;
            }
            case 7:
                processNextReservation();
                break;
            case 8:
                displayLastReturn();
                break;
            case 9:
                undoLastReturn();
                break;
            case 10:
                displayAllBorrowRecords();
                break;
            case 11:
                int userId;
                printf("Enter User ID to view all the borrowed books");
                scanf("%d" , &userId);
                displayAllBorrowByUser(userId); 
                break;
            case 12:
                return;
            default:
                printf("Invalid choice!\n");
        }
    } while (choice != 12);
}

/********************************************/
/* Browsing History Stack Functions         */
/********************************************/

// Initialize browsing history stack
void initBrowsingStack() {
    browsingHistory.top = NULL;
    browsingHistory.size = 0;

    returnStack = (rstack*)malloc(sizeof(rstack));
    if (returnStack == NULL) {
        printf("Memory allocation failed for return stack!\n");
        exit(1); // Exit if memory allocation fails
    }
    returnStack->top = NULL;
    returnStack->size = 0;
}

// Check if stack is empty
int isStackEmpty() {
    return browsingHistory.top == NULL;
}

// Push book to borrowing history
void pushToBrowsingHistory(int bookId) {
    StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));

    if (newNode == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }
    
    newNode->bookId = bookId;
    newNode->next = browsingHistory.top;
    browsingHistory.top = newNode;
    browsingHistory.size++;
    
    // Limit stack size
    if (browsingHistory.size > MAX_STACK_SIZE) {
        // Remove bottom node
        StackNode* current = browsingHistory.top;
        StackNode* prev = NULL;
        
        while (current->next != NULL) {
            prev = current;
            current = current->next;
        }
        
        if (prev != NULL) {
            prev->next = NULL;
            free(current);
            browsingHistory.size--;
        }
    }
}

// Pop book from browsing history
int popFromBrowsingHistory() {
    if (isStackEmpty()) {
        printf("Browsing history is empty!\n");
        return -1;
    }
    
    StackNode* temp = browsingHistory.top;
    int bookId = temp->bookId;
    
    browsingHistory.top = browsingHistory.top->next;
    free(temp);
    browsingHistory.size--;
    
    return bookId;
}

// Display browsing history
void displayBrowsingHistory() {
    if (isStackEmpty()) {
        printf("Browsing history is empty!\n");
        return;
    }
    
    StackNode* current = browsingHistory.top;
    int position = 1;
    
    printf("\n=== Browsing History ===\n");
    printf("---------------------------\n");
    
    while (current != NULL) {
        Book* book = searchBookById(bookRoot, current->bookId);
        if (book != NULL) {
            printf("%d. %s by %s\n", position, book->title, book->author);
        } else {
            printf("%d. [Deleted Book] (ID: %d)\n", position, current->bookId);
        }
        current = current->next;
        position++;
    }
}

/********************************************/
/* Library Structure (N-ary Tree) Functions */
/********************************************/

// Create a new library section
Section* createSection(const char* name) {
    Section* newSection = (Section*)malloc(sizeof(Section));
    if (newSection == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    
    strncpy(newSection->name, name, MAX_TITLE_LENGTH - 1);
    newSection->name[MAX_TITLE_LENGTH - 1] = '\0';
    newSection->firstChild = NULL;
    newSection->nextSibling = NULL;
    
    return newSection;
}

// Add child section to parent
void addChildSection(Section* parent, Section* child) {
    if (parent == NULL || child == NULL) {
        return;
    }
    
    if (parent->firstChild == NULL) {
        parent->firstChild = child;
    } else {
        Section* current = parent->firstChild;
        while (current->nextSibling != NULL) {
            current = current->nextSibling;
        }
        current->nextSibling = child;
    }
}

// Display library structure with proper indentation
void displayLibraryStructure(Section* root, int depth) {
    if (root == NULL) {
        return;
    }
    
    // Print current node with indentation
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    printf("└── %s\n", root->name);
    
    // Recursively display children
    displayLibraryStructure(root->firstChild, depth + 1);
    
    // Display siblings
    displayLibraryStructure(root->nextSibling, depth);
}

// Initialize library structure
void initializeLibraryStructure() {
    // Create root
    libraryRoot = createSection("Library");
    
    // Create main sections
    Section* computerScience = createSection("Computer Science Section");
    Section* literature = createSection("Literature Section");
    
    // Add main sections to root
    addChildSection(libraryRoot, computerScience);
    addChildSection(libraryRoot, literature);
    
    // Create subsections for Computer Science
    Section* outpatientCS = createSection("Outpatient Services");
    Section* referenceCS = createSection("Reference Section");
    
    // Add subsections to Computer Science
    addChildSection(computerScience, outpatientCS);
    addChildSection(computerScience, referenceCS);
    
    // Create subsections for Literature
    Section* inpatientLit = createSection("Inpatient Services");
    Section* archivesLit = createSection("Archives");
    
    // Add subsections to Literature
    addChildSection(literature, inpatientLit);
    addChildSection(literature, archivesLit);
}

// Add a new section to the library
void addSection() {
    char name[MAX_TITLE_LENGTH];
    char parentName[MAX_TITLE_LENGTH];
    
    printf("Enter section name: ");
    fgets(name, MAX_TITLE_LENGTH, stdin);
    name[strcspn(name, "\n")] = '\0';  // Remove newline
    
    printf("Enter parent section name (or 'library' for root): ");
    fgets(parentName, MAX_TITLE_LENGTH, stdin);
    parentName[strcspn(parentName, "\n")] = '\0';  // Remove newline
    
    Section* newSection = createSection(name);
    
    // Find parent section
    Section* queue[100];
    int front = 0, rear = 0;
    queue[rear++] = libraryRoot;
    
    Section* parent = NULL;
    while (front < rear) {
        Section* current = queue[front++];
        
        if (strcmp(current->name, parentName) == 0) {
            parent = current;
            break;
        }
        
        // Add children to queue
        Section* child = current->firstChild;
        while (child != NULL) {
            queue[rear++] = child;
            child = child->nextSibling;
        }
    }
    
    if (parent != NULL) {
        addChildSection(parent, newSection);
        printf("Section added successfully!\n");
    } else {
        printf("Parent section not found!\n");
        free(newSection);
    }
}

/********************************************/
/*         Search functionalities           */
/********************************************/

void searchMenu() {
    int choice;
    
    do {
        printf("\n=== Search Menu ===\n");
        printf("1. Search Book by ID\n");
        printf("2. Search Book by Title\n");
        printf("3. Search User by ID\n");
        printf("4. Search User by Name\n");
        printf("5. Back\n");
        printf("Choose an option: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: {
                int id;
                printf("Enter Book ID: ");
                scanf("%d", &id);
                Book* book = searchBookById(bookRoot, id);
                if (book != NULL) {
                    displayBook(book);
                } else {
                    printf("Book not found!\n");
                }
                break;
            }
            case 2: {
                char title[MAX_TITLE_LENGTH];
                printf("Enter Book Title (or part of it): ");
                while ((getchar()) != '\n'); // Clear input buffer
                fgets(title, MAX_TITLE_LENGTH, stdin);
                title[strcspn(title, "\n")] = '\0';

                Book* book = searchBookByTitle(bookRoot, title);
                if (book != NULL) {
                    displayBook(book);
                } else {
                    printf("No books found matching '%s'\n", title);
                }
                
                break;
            }
            case 3: {
                int id;
                printf("Enter User ID: ");
                scanf("%d", &id);
                User* user = searchUserById(id);
                if (user != NULL) {
                    displayUser(user);
                } else {
                    printf("User not found!\n");
                }
                break;
            }
            case 4: {
                char name[MAX_NAME_LENGTH];
                printf("Enter User Name: ");
                while ((getchar()) != '\n'); // Clear input buffer
                fgets(name, MAX_NAME_LENGTH, stdin);
                name[strcspn(name, "\n")] = '\0';
                User* user = searchUserByName(name);
                if (user != NULL) {
                    displayUser(user);
                } else {
                    printf("User not found!\n");
                }
                break;
            }
            case 5:
                return;
            default:
                printf("Invalid choice!\n");
        }
    } while (choice != 5);
}

void main(){
int choice;
initBrowsingStack();
do{
printf("=== Library Management System === ");
printf("\n1. Manage Books \n");
printf("2. Manage Users/Students\n");
printf("3. Manage Borrowed Books \n");
printf("4. Browse Library Sections\n");
printf("5. Search (Catalog, Student by ID/Name, Book by ID/Title\n");
printf("6. View Browsing History\n");
printf("7. Trees (Display Student Directory, Display Library Catalog, Library Structure, etc)\n");
printf("8. Save Data to File\n");
printf("9. Load Data from File\n");
printf("10. Exit \n");
scanf("%d" , &choice );
switch (choice)
{
case 1:
  manageBooks();
    break;
case 2:
  manageUsers();
    break;
case 3:
  manageBorrowing();
    break;
case 4:
    
    break;
case 5:
    searchMenu();
    break;
case 6:
    
    break;
case 7:
    
    break;
case 8:
    
    break;
case 9:
    
    break;
case 10:
printf("thanks for using our system");
    break;
default:
printf("please select a valid choice ");
    break;
}
}while(choice != 10);

}