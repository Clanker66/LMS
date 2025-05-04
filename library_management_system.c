
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_TITLE_LENGTH 100
#define MAX_AUTHOR_LENGTH 50
#define MAX_ISBN_LENGTH 20
#define MAX_NAME_LENGTH 50
#define MAX_ID_LENGTH 20
#define MAX_BOOKS 1000
#define MAX_USERS 500
#define MAX_STACK_SIZE 100
#define MAX_QUEUE_SIZE 100
#define MAX_BORROW_LIMIT 10
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

//Enum for System History
typedef enum {
USERADDED,
USERDELETED,
BOOKADDED,
BOOKDELETED
}History;
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
    int borrowCount;
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

// System History Stack Node
typedef struct HStackNode {
    Book* bookCopy;
    User* userCopy;
    time_t timeOfAction;
    History typeOfAction;
    struct HStackNode* next;
} HStackNode;

// Return History Stack Node
typedef struct RStackNode {
    int bookId;
    int userId;
    time_t returnDate;
    struct RStackNode* next;
} RStackNode;

// Return History Stack
typedef struct returnstack{
    int size ;
    RStackNode *top;
}RStack;

// Browsing History Stack
typedef struct {
    HStackNode* top;
    int size;
} HStack;

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
HStack* HistoryStack ;
BorrowRecord* borrowRecords = NULL;
RStack* returnStack;
int numbooks = 0;
int numofuser = 0;
int borrowCount = 0;


/********************************************/
/*     Function prototypes (needed )        */
/********************************************/
void pushToReturnHistory(int bookId , int userId);
void pushToSystemHistory(Book* book ,User* user ,History His );
/********************************************/
/*    BOOK MANAGEMENT set of Functions      */
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
    pushToSystemHistory(newBook , NULL , BOOKADDED);
    printf("Book added successfully!\n");
}

// Edit book details
void editBook() {
    char bname[MAX_TITLE_LENGTH];
        printf("Enter Book Title (or part of it): ");
        while ((getchar()) != '\n'); // Clear input buffer
        fgets(bname, MAX_TITLE_LENGTH, stdin);
        bname[strcspn(bname, "\n")] = '\0';
    
    Book* book = searchBookByTitle(bookRoot, bname);
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
    char title[MAX_TITLE_LENGTH];
        printf("Enter Book Title (or part of it): ");
        while ((getchar()) != '\n'); // Clear input buffer
        fgets(title, MAX_TITLE_LENGTH, stdin);
        title[strcspn(title, "\n")] = '\0';
    
    Book* book = searchBookByTitle(bookRoot, title);
    if (book == NULL) {
        printf("Book not found!\n");
        return;
    }
    
    displayBook(book);
}

// Delete a book
void deleteBookMenu() {
    char title[MAX_TITLE_LENGTH];
        printf("Enter Book Title (or part of it): ");
        while ((getchar()) != '\n'); // Clear input buffer
        fgets(title, MAX_TITLE_LENGTH, stdin);
        title[strcspn(title, "\n")] = '\0';
    
    Book* book = searchBookByTitle(bookRoot, title);
    if (book == NULL) {
        printf("Book not found!\n");
        return;
    }
    
    // Check if the book is borrowed
    if (book->status != AVAILABLE) {
        printf("Cannot delete book as it is currently borrowed or reserved!\n");
        return;
    }
    pushToSystemHistory(book , NULL , BOOKDELETED);
    bookRoot = deleteBook(bookRoot, book->id);
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
    newUser->borrowCount = 0;
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
    pushToSystemHistory(NULL,current,USERDELETED);
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
    pushToSystemHistory(NULL,newUser,USERADDED);
    printf("User added successfully!\n");
}

// Edit user details
void editUser() {

    char uname[MAX_NAME_LENGTH];
    printf("Enter User's name: ");
    while ((getchar()) != '\n');
    fgets(uname, MAX_NAME_LENGTH, stdin);
    uname[strcspn(uname, "\n")] = '\0';

    User* user = searchUserByName(uname);
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
    char name[MAX_NAME_LENGTH];
    printf("Enter User's name: ");
    while ((getchar()) != '\n');
    fgets(name, MAX_NAME_LENGTH, stdin);
    name[strcspn(name, "\n")] = '\0';

    User* user = searchUserByName(name);
    if (user == NULL) {
        printf("User not found!\n");
        return;
    }
    
    displayUser(user);
}

// Delete user menu function
void deleteUser() {
    char name[MAX_NAME_LENGTH];
    printf("Enter User's name: ");
    while ((getchar()) != '\n');
    fgets(name, MAX_NAME_LENGTH, stdin);
    name[strcspn(name, "\n")] = '\0';

    User* user = searchUserByName(name);
    if (user == NULL) {
        printf("User not found!\n");
        return;
    }
    
    // Check if user has borrowed books
    BorrowRecord* current = borrowRecords;
    while (current != NULL) {
        if (current->userId == user->id && !current->returned) {
            printf("Cannot delete user as they have borrowed books!\n");
            return;
        }
        current = current->next;
    }
    
    delusernode(user->id);
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
    User* user = searchUserById(userId);
    while (current != NULL) {
        if (current->bookId == bookId && current->userId == userId && !current->returned) {
            current->returned = true;
            current->returnDate = time(NULL);
            if(difftime(current->dueDate , time(NULL)) < 0){
                user->status=SUSPENDED;
            }
            
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
            user->borrowCount--;
            pushToReturnHistory(bookId,userId);
            printf("Book returned successfully!\n");
            return;
        }
        current = current->next;
    }
    
    printf("No matching borrow record found!\n");
}

//adds returns to stack
void pushToReturnHistory(int bookId , int userId) {
    RStackNode* newNode = (RStackNode*)malloc(sizeof(RStackNode));
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
        RStackNode* current = returnStack->top;
        RStackNode* prev = NULL;
        
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

//view last return
void displayLastReturn() {

    Book* returnedBook = searchBookById(bookRoot , returnStack->top->bookId);
    User* user = searchUserById(returnStack->top->userId);
    struct tm* rettime;
    rettime = localtime(&(returnStack->top->returnDate));
    printf("%s has been returned by %s on %s" , returnedBook->title , user->name , asctime(rettime));

}

//undo last return 
void undoLastReturn(){

Book* book = searchBookById(bookRoot , returnStack->top->bookId);


BorrowRecord* current = borrowRecords;

while(current != NULL ){
    if(current->bookId == returnStack->top->bookId && current->userId == returnStack->top->userId){

        if(book->status == AVAILABLE){

            current->returnDate = 0;
            current->returned = false;

            printf("the return was undone successfully \n");

            struct tm* rettime;
            rettime = localtime(&(current->dueDate));
            book->status =BORROWED;

            printf("your book return window is still due until %s \n" , asctime(rettime));
        }
        else{
            printf("the book you wish to unreturn has been loaned out \n ");
            printf("do you want to enter the queue for this book ? (1-Yes/0-No): ");

            int choice ;
            scanf("%d" , &choice);

            if (choice == 1) {
                // Initialize queue if not initialized
                if (bookQueues[book->id].size == 0) {
                    initializeBookQueue(book->id);
                }
                
                // Add user to queue
                enqueueUser(book->id, returnStack->top->userId);
                printf("\n User added to reservation queue!\n");
            }
        }
        break;
    }
    current = current->next;
}
if(current == NULL){
    printf("no recent returns");
    return ;
}
//pop the top
RStackNode* temp;
temp = returnStack->top;
returnStack->top = returnStack->top->next;
returnStack->size--;
free(temp);

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

    char title[MAX_TITLE_LENGTH],name[MAX_NAME_LENGTH];

    
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

    if(user->status != ACTIVE || user->borrowCount > MAX_BORROW_LIMIT){
        printf("user is not eligible to borrow this book");
        return ;
    }

    // Check if book is available
    if (book->status == AVAILABLE) {
        // Create borrow record
        BorrowRecord* newRecord = createBorrowRecord(user->id, book->id);
        addBorrowRecord(newRecord);
        
        // Update book status
        book->status = BORROWED;
        
        user->borrowCount++;
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

    char title[MAX_TITLE_LENGTH];
        printf("Enter Book Title (or part of it): ");
        while ((getchar()) != '\n'); // Clear input buffer
        fgets(title, MAX_TITLE_LENGTH, stdin);
        title[strcspn(title, "\n")] = '\0';
    
        char name[MAX_NAME_LENGTH];
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
    
    // Initialize queue if not initialized
    if (bookQueues[book->id].size == 0) {
        initializeBookQueue(book->id);
    }

    //checks if user is already in queue 
    int Uid;bool exist = false ;
    for(int i=0 ; i<bookQueues[book->id].size ; i++){
        Uid = dequeueUser(book->id);

        if(Uid == user->id){
        exist = true;
        }

        enqueueUser(book->id , Uid);
    }
    if(exist){
        printf("the user is already in the reservation queue");
        return;
    }

    // Add user to queue
    enqueueUser(book->id, user->id);
    
    // If book is available, mark as reserved
    if (book->status == AVAILABLE) {
        book->status = RESERVED;
        printf("Book reserved successfully! You can pick it up now.\n");
    } else {
        printf("User added to reservation queue!\n");
        printf("Your position in queue: %d\n", bookQueues[book->id].size);
    }
}

// Cancel reservation 
void cancelReservation() {
    int userId;
    char title[MAX_TITLE_LENGTH];
        printf("Enter Book Title (or part of it): ");
        while ((getchar()) != '\n'); // Clear input buffer
        fgets(title, MAX_TITLE_LENGTH, stdin);
        title[strcspn(title, "\n")] = '\0';
    
        printf("Enter User ID: ");
        scanf("%d", &userId);
        
        // Validate book    
    Book* book = searchBookByTitle(bookRoot, title);
    if (book == NULL) {
        printf("Book not found!\n");
        return;
    }
    
    if (bookQueues[book->id].size == 0 && book->status == RESERVED) {
        book->status = AVAILABLE;
        printf("Book is available for borrowing");
        return ;
    }

    // Check if queue exists
    if (bookQueues[book->id].size == 0) {
        printf("No reservations for this book!\n");
        return;
    }
    
    // Find and remove user from queue
    int Uid;bool exist = false ;
    for(int i=0 ; i<bookQueues[book->id].size ; i++){
        Uid = dequeueUser(book->id);

        if(Uid == userId){
        exist = true;
        }

        if(!exist){
        enqueueUser(book->id , Uid);
        }
    }
    if(exist){
        bookQueues[book->id].size--;
        printf("Reservation cancelled successfully!\n");
    }
    }

// Check book availability
void checkBookAvailability() {
    char title[MAX_TITLE_LENGTH];
        printf("Enter Book Title (or part of it) to check : ");
        while ((getchar()) != '\n'); // Clear input buffer
        fgets(title, MAX_TITLE_LENGTH, stdin);
        title[strcspn(title, "\n")] = '\0';
    
    Book* book = searchBookByTitle(bookRoot, title);

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
                if (current->bookId == book->id&& !current->returned) {
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
            if (bookQueues[book->id].size > 0) {
                printf("Reservation Queue Length: %d\n", bookQueues[book->id].size);
            }
            break;
        case RESERVED:
            printf("Reserved\n");
            
            // Show first person in queue
            if (bookQueues[book->id].front != NULL) {
                User* user = searchUserById(bookQueues[book->id].front->userId);
                if (user != NULL) {
                    printf("Reserved for: %s\n", user->name);
                }
                
                // Show queue length
                if (bookQueues[book->id].size > 1) {
                    printf("Additional Reservations: %d\n", bookQueues[book->id].size - 1);
                }
            }
            break;
    }
}

// Process next reservation
void processNextReservation() {
    char title[MAX_TITLE_LENGTH];
        printf("Enter Book Title (or part of it): ");
        while ((getchar()) != '\n'); // Clear input buffer
        fgets(title, MAX_TITLE_LENGTH, stdin);
        title[strcspn(title, "\n")] = '\0';
    
    Book* book = searchBookByTitle(bookRoot, title);

    if (book == NULL) {
        printf("Book not found!\n");
        return;
    }
    
    if (book->status != AVAILABLE && book->status != RESERVED) {
        printf("Book is not available for reservation processing!\n");
        return;
    }
    
    if (isQueueEmpty(book->id)) {
        printf("No reservations in queue for this book!\n");
        return;
    }
    
    // Dequeue next user
    int userId = dequeueUser(book->id);
    if (userId == -1) {
        return;
    }
    
    User* user = searchUserById(userId);
    if (user == NULL) {
        printf("User not found!\n");
        return;
    }
    
    // Create borrow record
    BorrowRecord* newRecord = createBorrowRecord(userId, book->id);
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
    char title[MAX_TITLE_LENGTH];
        printf("Enter Book Title (or part of it): ");
        while ((getchar()) != '\n'); // Clear input buffer
        fgets(title, MAX_TITLE_LENGTH, stdin);
        title[strcspn(title, "\n")] = '\0';

        char name[MAX_NAME_LENGTH];
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
    
    // Check if book is borrowed
    if (book->status != BORROWED) {
        printf("This book is not currently borrowed!\n");
        return;
    }
    
    // Verify the user has borrowed this book
    BorrowRecord* current = borrowRecords;
    bool userHasBorrowed = false;
    while (current != NULL) {
        if (current->bookId == book->id && current->userId == user->id && !current->returned) {
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
    returnBook(book->id, user->id);
    
    // Check if there are users in queue
    if (!isQueueEmpty(book->id)) {
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
        if(current->userId == userId && current->returned == false);
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
        printf("\n");

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
                char title[MAX_TITLE_LENGTH];
                printf("Enter Book Title (or part of it): ");
                while ((getchar()) != '\n'); // Clear input buffer
                fgets(title, MAX_TITLE_LENGTH, stdin);
                title[strcspn(title, "\n")] = '\0';
    
                Book* book = searchBookByTitle(bookRoot, title);
                displayBookQueue(book->id);
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
            //doesnt work
            char name[MAX_NAME_LENGTH];
            printf("Enter User's name: ");
            while ((getchar()) != '\n');
            fgets(name, MAX_NAME_LENGTH, stdin);
            name[strcspn(name, "\n")] = '\0';
        
            User* user = searchUserByName(name);

            if(user == NULL){
                printf("user unfound");
                break;
            }
                displayAllBorrowByUser(user->id); 
                break;
            case 12:
                return;
            default:
                printf("Invalid choice!\n");
        }
    } while (choice != 12);
}

/********************************************/
/*             Stack Functions              */
/********************************************/

// Initialize stacks
void initStacks() {
    HistoryStack = (HStack*)malloc(sizeof(HStack));
    if (HistoryStack == NULL) {
        printf("Memory allocation failed for return stack!\n");
        exit(1); // Exit if memory allocation fails
    }
    HistoryStack->top = NULL;
    HistoryStack->size = 0;

    returnStack = (RStack*)malloc(sizeof(RStack));
    if (returnStack == NULL) {
        printf("Memory allocation failed for return stack!\n");
        exit(1); // Exit if memory allocation fails
    }
    returnStack->top = NULL;
    returnStack->size = 0;
}

// Check if history stack is empty
int isHStackEmpty() {
    return HistoryStack->top == NULL;
}

//check if return stack is empty
int isRStackEmpty(){
    return returnStack->top == NULL;
}

// most recent history to stack
void pushToSystemHistory(Book* book , User* user ,History His) {
    HStackNode* newNode = (HStackNode*)malloc(sizeof(HStackNode));

    if (newNode == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }
    switch (His)
    {
    case USERADDED:
    newNode->userCopy = user;
    newNode->bookCopy = book;
    newNode->typeOfAction = His;
    newNode->timeOfAction =time(NULL);
    newNode->next = HistoryStack->top;
    HistoryStack->top = newNode;
    HistoryStack->size++;
        break;
    case USERDELETED:
    newNode->userCopy = user;
    newNode->bookCopy = book;
    newNode->typeOfAction = His;
    newNode->timeOfAction =time(NULL);
    newNode->next = HistoryStack->top;
    HistoryStack->top = newNode;
    HistoryStack->size++;
        break;
    case BOOKADDED:
    newNode->userCopy = user;
    newNode->bookCopy = book;
    newNode->typeOfAction = His;
    newNode->timeOfAction =time(NULL);
    newNode->next = HistoryStack->top;
    HistoryStack->top = newNode;
    HistoryStack->size++;
        break;
    case BOOKDELETED:
    newNode->userCopy = user;
    newNode->bookCopy = book;
    newNode->typeOfAction = His;
    newNode->timeOfAction =time(NULL);
    newNode->next = HistoryStack->top;
    HistoryStack->top = newNode;
    HistoryStack->size++;
        break;
    default:
    printf("not a valid action");
        break;
    }
    
    
    // Limit stack size
    if ( HistoryStack->size > MAX_STACK_SIZE) {
        // Remove bottom node
        HStackNode* current = HistoryStack->top;
        HStackNode* prev = NULL;
        
        while (current->next != NULL) {
            prev = current;
            current = current->next;
        }
        
        if (prev != NULL) {
            prev->next = NULL;
            free(current);
            HistoryStack->size--;
        }
    }
}

// undo most recent action
void undoSystemhistory() {
    if (isHStackEmpty()) {
        printf("System history is empty!\n");
        return ;
    }

    switch (HistoryStack->top->typeOfAction)
    {
    case USERADDED:{
    int id = HistoryStack->top->userCopy->id;
    BorrowRecord* current = borrowRecords;
    while (current != NULL) {
        if (current->userId == id && !current->returned) {
            printf("Cannot delete user as they have borrowed books!\n");
            return;
        }
        current = current->next;
    }
    
    delusernode(id);
    printf("Actions undone successfully");
}
        break;
    case USERDELETED:{
    int id = HistoryStack->top->userCopy->id , age = HistoryStack->top->userCopy->age;
    char gender = HistoryStack->top->userCopy->gender,name[MAX_NAME_LENGTH] , uid[MAX_ID_LENGTH];
    strcpy(name , HistoryStack->top->userCopy->name);
    strcpy(uid , HistoryStack->top->userCopy->user_id);
    User* user = (User*)malloc(sizeof(User));
    user =createUser(id, name , uid , age , gender);
    addUserToList(user);
    printf("Action undone successfully");
    }
        break;
    case BOOKADDED:{
        deleteBook(bookRoot , HistoryStack->top->bookCopy->id);
        printf("Action undone successfully");
    }
        break; 
    case BOOKDELETED:{
        int id = HistoryStack->top->bookCopy->id;
        char title[MAX_TITLE_LENGTH] , author[MAX_AUTHOR_LENGTH] , isbn[MAX_ISBN_LENGTH]; 
        strcpy(title ,HistoryStack->top->bookCopy->title);
        strcpy(author ,HistoryStack->top->bookCopy->author);
        strcpy(isbn ,HistoryStack->top->bookCopy->isbn);
        Book* newBook = addBook(id, title, author, isbn);
        bookRoot = insertBook(bookRoot, newBook);
        printf("Action undone successfully");
    }
        break;
    default:
    printf("not a valid action");
        break;
    }
}

// Display history
void displaySystemHistory() {
    if (isHStackEmpty()) {
        printf("System history is empty!\n");
        return;
    }
    
    HStackNode* current = HistoryStack->top;
    int position = 1 , i = 0 ;
     uint8_t cont = 1 ;
     struct tm* acttime ;
    printf("\n=== System History ===\n");
    printf("------------------------\n");
    do{
    while (current != NULL && i%6 != 0 || i == 0){
        switch (current->typeOfAction)
        {
        case USERADDED:
        acttime= localtime(&(current->timeOfAction));
        printf("%d - A user going by the name of %s has been added on :\n %s " ,position, current->userCopy->name ,asctime(acttime) );
            break;
        case USERDELETED:
        acttime= localtime(&(current->timeOfAction));
        printf("%d - A user going by the name of %s was deleted on :\n %s " ,position, current->userCopy->name ,asctime(acttime) );
            break;
        case BOOKADDED:
        acttime= localtime(&(current->timeOfAction));
        printf("%d - A book with the title of %s has been added on :\n %s " ,position , current->bookCopy->title , asctime(acttime) );
            break;
        case BOOKDELETED:
        acttime= localtime(&(current->timeOfAction));
        printf("%d - A book with the title of %s was deleted on :\n %s ",position , current->bookCopy->title ,asctime(acttime) );
            break;
        default:
        printf("not a valid action");
            break;
        }
        printf("------------------------\n");
        current = current->next;
        position++;
        i++;
    }
    printf("do you wish to show more ?(1-Yes/0-No):");
    scanf("%u" , &cont);
    i = 0;
    }while(cont);
}

//history Menu
void HistoryMenu(){
int choice;
do{
    printf("\n=== History Menu ===\n");
    printf("1. View Recent History\n");
    printf("2. undo Most recent Action\n");
    printf("3. Back\n");
    printf("Choose an option: ");
    scanf("%d", &choice);

    switch (choice)
    {
    case 1:
        displaySystemHistory();
        break;
    case 2:
        undoSystemhistory();
        break;
    case 3:
        return;
        break; 
    default:
        printf("please input a valid choice");
        break;
    }

}while(choice != 3);

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

/********************************************/
/*              Main function               */
/********************************************/
void main(){
int choice;
initStacks();
do{
printf("=== Library Management System === ");
printf("\n1. Manage Books \n");
printf("2. Manage Users/Students\n");
printf("3. Manage Borrowed Books \n");
printf("4. Search (Catalog, Student by ID/Name, Book by ID/Title \n");
printf("5. View System History \n");
printf("6. Display Directories \n");
printf("7. Save Data to File\n");
printf("8. Load Data from File\n");
printf("9. Exit \n");
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
  searchMenu();
    break;
case 5:
  HistoryMenu();
    break;
case 6:
    
    break;
case 7:
    
    break;
case 8:
    
    break;
case 9:
printf("thanks for using our system");
    break;
default:
printf("please select a valid choice ");
    break;
}
}while(choice != 9);

}