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

## Feature 3 Questions:

### 1. "Down then across" columnar format logic
"Down then across" format mein hum files ko columns mein vertically fill karte hain. Pehle column 1 ko fill karte hain, phir column 2, etc. Simple loop insufficient hai kyunki humein pehle calculate karna padta hai kitne columns aur rows banenge. Phir har row ke liye, har column se ek item print karna padta hai.

### 2. ioctl system call purpose
ioctl system call terminal ki width detect karne ke liye use hota hai. Fixed-width fallback (80 columns) use karne se program terminal size ke according adjust nahi kar payega. Chote terminal mein files cut ho jayengi, bade terminal mein extra space waste hoga.


## Feature 4 Questions:

### 1. Vertical vs Horizontal printing complexity
"Down then across" (vertical) printing more complex hai kyunki isme pehle calculate karna padta hai kitne columns aur rows banenge, phir mathematical formula use karke correct index find karna padta hai. "Across" (horizontal) printing simpler hai kyunki hum simply left-to-right print karte hain aur jab line full ho jaye toh new line start kar dete hain.

### 2. Display mode management strategy
Maine ek integer flag `display_mode` use kiya: 0=column, 1=long, 2=horizontal. getopt() loop mein options check hote hain aur flag set ho jata hai. list_directory() function mein ye flag check karke appropriate display function call hota hai.
