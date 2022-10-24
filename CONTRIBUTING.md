# CONTRIBUTING TO CPP-ENGINE

If you wish to contribute to cpp-engine, please take note of the following information.

cpp-engine is a small hobby project by one person. While this may eventually change, for now it means:
- Contributions are by no means expected

And 
- The repository is not very user friendly and contributing may be more difficult than most open source repositories

### General guide to contributing to cpp-engine:

For bugs and suggestions, these should be reported via an [issue](https://github.com/Bobscorn/cpp-engine).

To solve an open issue:
- Fork this repo
- Create a new branch for your work
- Commit changes to your fork's branch
- Open a Pull Request and properly fill in the Pull Request template.
- Complete any requests by reviewers

Otherwise a few things to keep in mind:
- Make sure pull requests resolve an open issue. Your pull request may likely be denied if it does not.
- Pull requests should only attempt to resolve 1 issue at a time, and ideally require as little modification as possible.
- Pull requests must successfully build

## Styling

Styling is up to the maintainer(s) to enforce, and they may be nitpicky, or not.
You may very well get away with following different style guidelines than presented here.

You may also find that these guidelines are not followed in the code as is. Some of this code is fairly old, written by a lesser experienced developer (past me).

In general these are the rules to follow:

### class/struct/enum types should be named with Pascal Case. For eg. `class MyUIClass;`
  
### Functions and Member Functions should also be named with Pascal Case. For eg. `MyResult MyKillFunction(Player* player, Entity* killer);`

### If a type is POD (Plain-ol-data) it should be a struct with no private or protected members. It may have functions, including those that mutate.

### If a type is not POD (Plain-ol-data) or has protected or private members, it should be a class.

### Protected and private member variables should be prefixed with an m_. For eg. `protected: int m_LastIndex;` or `private: MyType m_MyInfo;`

### Non-Member variables should be named with lower-case camel case. For eg. `int index = 0;` or `std::vector<MyType> myContainer;`

### Asterisks and ampersands used for pointers and references should be on the left. For eg. `Player* myPlayer;` or `const Player& myReadonlyPlayer;`

### Curly Parentheses/Braces should start on a new line, unless a single line is reasonable.

For eg:
```cpp
void MyFunction(MyTypeA a, const MyTypeB& b)
{
  // Do stuff
}
```

Or a single line:
```cpp
void MyFunction(MyTypeA a, const MyTypeB& b) { /* do very simple single liner*/}
```
Examples of reasonable single liners involve:
- Calling a single function with or without parameters
- Returning a simple output

Examples of acceptable calls: `... { DoFunc(myPlayer); }` or `... { glBindProgram(program.Get()); }`

Examples of unacceptable single line calls: `... { DoFunc(myPlayer); DeleteResource(myPlayer->Feet.Get()); }`

Examples of acceptable single line returns are `... { return 0; }` or `... { return a + b; }` or `... { return a.subtr(b); }`

Examples of unacceptable single line returns are `... { return a + b * (pow(c >> 2, i)); }` or `... { return a.substr(b + 3, (c.substr(b, 3)); }`

General rule of thumb is no more than 2 operations or function calls.



