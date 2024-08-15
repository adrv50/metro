# Metro: a simple typed interpreter language.

# How build
```
$ ./build.sh
```

## Example
```
use io;

fn fibo(n: int) -> int {
    if n < 2 {
        return 1;
    }

    return fibo(n - 2) + fibo(n - 1);
}

io.println("fibo(10) = ", fubo(10));
```