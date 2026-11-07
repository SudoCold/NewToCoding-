/*
Problem: Build a Library Lending System

Design an object-oriented system for a library that lends books to members.

Requirements:
- Each book has a title, author, and ISBN. The library may own multiple copies of the same book.
- Each physical copy has a unique ID and can be available, borrowed, or lost.
- A member can borrow up to three copies at a time.
- Each loan lasts 14 days.
- Members can reserve a book when all its copies are unavailable. Reservations are fulfilled in order.
- When a copy is returned, hold it for the next member in the reservation queue, if any.
- Prevent borrowing an unavailable copy, returning a copy with no active loan, and reserving the same book twice.
- A member who already has an overdue loan cannot borrow another copy.
*/
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <deque>
#include <stdexcept>
using namespace std;

enum CopyStatus {
    AVAILABLE,
    BORROWED,
    HELD,
    LOST
};

class Book {
public:
    string isbn;
    string title;
    string author;
    deque<int> reservations; // Member IDs, in FIFO order

    Book() {}

    Book(const string& i, const string& t, const string& a)
        : isbn(i), title(t), author(a) {}
};

class BookCopy {
public:
    int id;
    string isbn;
    CopyStatus status;
    int heldFor;

    BookCopy() : id(0), status(AVAILABLE), heldFor(-1) {}

    BookCopy(int copyId, const string& bookIsbn)
        : id(copyId), isbn(bookIsbn),
          status(AVAILABLE), heldFor(-1) {}
};

class Member {
public:
    int id;
    string name;

    Member() : id(0) {}

    Member(int memberId, const string& memberName)
        : id(memberId), name(memberName) {}
};

class Loan {
public:
    int copyId;
    int memberId;
    int borrowedDay;
    int dueDay;

    Loan()
        : copyId(0), memberId(0), borrowedDay(0), dueDay(0) {}

    Loan(int copy, int member, int day)
        : copyId(copy), memberId(member),
          borrowedDay(day), dueDay(day + 14) {}

    bool isOverdue(int today) const {
        return today > dueDay;
    }
};

class Library {
private:
    map<string, Book> books;
    map<int, BookCopy> copies;
    map<int, Member> members;
    map<int, Loan> activeLoans; // Key: copy ID

    Book& getBook(const string& isbn) {
        map<string, Book>::iterator it = books.find(isbn);

        if (it == books.end())
            throw runtime_error("Book not found.");

        return it->second;
    }

    BookCopy& getCopy(int copyId) {
        map<int, BookCopy>::iterator it = copies.find(copyId);

        if (it == copies.end())
            throw runtime_error("Copy not found.");

        return it->second;
    }

    void requireMember(int memberId) const {
        if (members.find(memberId) == members.end())
            throw runtime_error("Member not found.");
    }

    int loanCount(int memberId) const {
        int count = 0;

        map<int, Loan>::const_iterator it;
        for (it = activeLoans.begin(); it != activeLoans.end(); ++it) {
            if (it->second.memberId == memberId)
                ++count;
        }

        return count;
    }

    bool hasOverdueLoan(int memberId, int today) const {
        map<int, Loan>::const_iterator it;

        for (it = activeLoans.begin(); it != activeLoans.end(); ++it) {
            if (it->second.memberId == memberId &&
                it->second.isOverdue(today)) {
                return true;
            }
        }

        return false;
    }

    bool hasReservation(const Book& book, int memberId) const {
        deque<int>::const_iterator reservation;

        for (reservation = book.reservations.begin();
             reservation != book.reservations.end();
             ++reservation) {
            if (*reservation == memberId)
                return true;
        }

        // A fulfilled reservation remains active while its copy is held.
        map<int, BookCopy>::const_iterator copy;

        for (copy = copies.begin(); copy != copies.end(); ++copy) {
            if (copy->second.isbn == book.isbn &&
                copy->second.status == HELD &&
                copy->second.heldFor == memberId) {
                return true;
            }
        }

        return false;
    }

    void makeAvailableOrHold(BookCopy& copy) {
        Book& book = getBook(copy.isbn);

        if (book.reservations.empty()) {
            copy.status = AVAILABLE;
            copy.heldFor = -1;
        } else {
            copy.status = HELD;
            copy.heldFor = book.reservations.front();
            book.reservations.pop_front();
        }
    }

public:
    void addBook(const string& isbn,
                 const string& title,
                 const string& author) {
        if (books.find(isbn) != books.end())
            throw runtime_error("ISBN already exists.");

        books.insert(make_pair(isbn, Book(isbn, title, author)));
    }

    void addCopy(int copyId, const string& isbn) {
        getBook(isbn);

        if (copies.find(copyId) != copies.end())
            throw runtime_error("Copy ID already exists.");

        copies.insert(make_pair(copyId, BookCopy(copyId, isbn)));
        makeAvailableOrHold(getCopy(copyId));
    }

    void addMember(int memberId, const string& name) {
        if (members.find(memberId) != members.end())
            throw runtime_error("Member ID already exists.");

        members.insert(make_pair(memberId, Member(memberId, name)));
    }

    void borrowCopy(int memberId, int copyId, int today) {
        requireMember(memberId);
        BookCopy& copy = getCopy(copyId);

        if (loanCount(memberId) >= 3)
            throw runtime_error("Borrowing limit reached.");

        if (hasOverdueLoan(memberId, today))
            throw runtime_error("Member has an overdue loan.");

        bool available = copy.status == AVAILABLE;
        bool heldForMember =
            copy.status == HELD && copy.heldFor == memberId;

        if (!available && !heldForMember)
            throw runtime_error("Copy is unavailable to this member.");

        activeLoans.insert(
            make_pair(copyId, Loan(copyId, memberId, today))
        );

        copy.status = BORROWED;
        copy.heldFor = -1;
    }

    void returnCopy(int copyId, int today) {
        BookCopy& copy = getCopy(copyId);
        map<int, Loan>::iterator loan = activeLoans.find(copyId);

        if (loan == activeLoans.end())
            throw runtime_error("Copy has no active loan.");

        if (today < loan->second.borrowedDay)
            throw runtime_error("Return date precedes borrowing date.");

        activeLoans.erase(loan);
        makeAvailableOrHold(copy);
    }

    void reserveBook(int memberId, const string& isbn) {
        requireMember(memberId);
        Book& book = getBook(isbn);

        if (hasReservation(book, memberId))
            throw runtime_error("Member already reserved this book.");

        map<int, BookCopy>::const_iterator it;

        for (it = copies.begin(); it != copies.end(); ++it) {
            if (it->second.isbn == isbn &&
                it->second.status == AVAILABLE) {
                throw runtime_error(
                    "A copy is available; borrow it instead."
                );
            }
        }

        book.reservations.push_back(memberId);
    }

    void markLost(int copyId) {
        BookCopy& copy = getCopy(copyId);

        if (copy.status == LOST)
            throw runtime_error("Copy is already lost.");

        // Restore a held reservation to the front of the queue.
        if (copy.status == HELD) {
            getBook(copy.isbn).reservations.push_front(copy.heldFor);
        }

        // An existing loan remains active until the lost copy is returned.
        copy.status = LOST;
        copy.heldFor = -1;
    }

    void printCopy(int copyId) {
        const BookCopy& copy = getCopy(copyId);

        cout << "Copy " << copy.id << ": ";

        switch (copy.status) {
            case AVAILABLE:
                cout << "Available";
                break;
            case BORROWED:
                cout << "Borrowed";
                break;
            case HELD:
                cout << "Held for member " << copy.heldFor;
                break;
            case LOST:
                cout << "Lost";
                break;
        }

        cout << endl;
    }
};

int main() {
    try {
        Library library;

        library.addBook("978-0001", "The C++ Book", "A. Writer");
        library.addCopy(101, "978-0001");

        library.addMember(1, "Alice");
        library.addMember(2, "Bob");

        library.borrowCopy(1, 101, 0);
        library.reserveBook(2, "978-0001");

        library.returnCopy(101, 10);
        library.printCopy(101); // Held for member 2

        library.borrowCopy(2, 101, 11);
        library.printCopy(101); // Borrowed

        library.returnCopy(101, 20);
        library.printCopy(101); // Available
    } catch (const exception& error) {
        cerr << "Error: " << error.what() << endl;
        return 1;
    }

    return 0;
}
