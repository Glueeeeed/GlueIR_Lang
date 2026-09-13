# Code Samples

This section contains working code examples illustrating various features and idioms in the Glue programming language.

---

## 1. Complete Application (`hello.glue`)

Demonstrates functions, `const` and `sticky` variables, loop iterations, and multi-argument `shout` logging.

```c
/*g
  Hello World and feature demo in Glue
g*/

func string getGreeting() {
    return "Hello from Glue Language!";
}

func printDivider() {
    shout("---------------------------------");
}

func int main() {
    printDivider();
    shout(getGreeting());
    printDivider();

    const double PI = 3.14159;
    sticky string status = "initialized";

    shout("Math constant PI: ", PI);
    shout("Initial status: ", status);

    status = "running";
    shout("Updated status: ", status);

    int count = 0;
    while (count < 3) {
        shout("Iteration step: ", count);
        count = count + 1;
    }

    printDivider();
    return 0;
}
```

---

## 2. Sticky Variable Mutability (`sticky.glue`)

Demonstrates how `sticky` variables permit exactly one reassignment after declaration.

```c
func int main() {
    sticky int port = 8080;
    shout("Initial port: ", port);

    // First reassignment is permitted:
    port = 9000;
    shout("Updated port: ", port);

    // Uncommenting the following line will fail compilation:
    // port = 9090; // Semantic error: variable 'port' is 'sticky' and has already been reassigned once

    return 0;
}
```

---

## 3. Mathematical Operations & Comparisons (`expressions.glue`)

Demonstrates operator precedence, boolean comparisons, and type promotion.

```c
func int main() {
    int a = 10;
    int b = 20;
    double factor = 2.5;

    // Automatic type promotion (int + double -> double)
    double result = a + b * factor;
    shout("Calculated result: ", result);

    bool isHigher = result > 50.0;
    if (isHigher == true) {
        shout("Result exceeds threshold.");
    } else {
        shout("Result is within threshold.");
    }

    return 0;
}
```

---

## 4. Benchmark Loop (`bench.glue`)

Measures high-iteration loop performance compiled natively via LLVM.

```c
func int main() {
    int i = 0;
    int max = 100000000;

    while (i < max) {
        i = i + 1;
    }

    shout("Iterations completed: ", i);
    return 0;
}
```


## 5. Game (`game.glue`)

Measures using if statements, while, functions with return values, and more for console game.

```c
func int generateRandomNumber(int a,int b) {
    int number = random(a,b);
    return number;
}

func int main() {
    boolean isEnd = false;
    int lives = 3;
    int number = generateRandomNumber(1,50);

    while(!isEnd) {

        shout("Enter number: ");
        int result = toInt(shin());

        if (lives <= 0) {
            shout("Game over!");
            isEnd = true;
        }

        if (result == number) {
            shout("Correct! Guessed number is ", result);
            isEnd = true;
        } else {
            lives = lives - 1;

            if (lives == 0) {
              shout("Incorrect! Try again. You have last chance");
            } else {
              shout("Incorrect! Try again. You have ", lives , " lives");
            }
        }
    }

 
   return 0;
}
```
