# Assignment Report

## Feature 2 Questions:

### 1. Difference between stat() and lstat()
stat() aur lstat() dono file metadata retrieve karte hain, lekin lstat() symbolic links ke liye alag behave karta hai. stat() symbolic link follow karke target file ki information deta hai, jabki lstat() symbolic link ki khud ki information deta hai. ls command mein lstat() use karna better hai kyunki humein symbolic links ko as links dikhana hai, na ki unke target files ko.

### 2. st_mode field extraction using bitwise operators
st_mode field mein file type aur permissions dono store hote hain. Bitwise operators use karke hum inhe extract kar sakte hain:

- File type check: `(mode & S_IFMT) == S_IFDIR` - directory check
- Permissions check: `(mode & S_IRUSR)` - user read permission
- `(mode & S_IWUSR)` - user write permission  
- `(mode & S_IXUSR)` - user execute permission

Macros use karna easier hai: S_ISDIR(mode), S_ISREG(mode), etc.
