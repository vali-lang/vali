
<div align="center">
<p>
    <img width="170" src="https://raw.githubusercontent.com/valk-lang/valk/master/misc/valk.svg">
</p>
</div>

# Valk programming language

[Website](https://valk-lang.dev) | [Documentation](https://github.com/valk-lang/valk/blob/main/docs/docs.md) | [Roadmap](https://github.com/valk-lang/valk/blob/main/ROADMAP.md) | [Discord](https://discord.gg/RwEGqdSERA)

Valk is a programming language aimed to be fast & simple at the same time. It can be used for high & mid level programming. Valk is unique because of its new way of doing garbage collection. Its runtime is much faster than go and in some cases rust, while also using less memory. On top of that, a GC allows us to keep the language very simple like python. You get the best of both worlds.

**Features**: Super fast non-pausing GC ⚡, No undefined behaviour, Great package management, Generics, Fast compile times, Cross compiling, linking c-libraries.

**Valk is still in alpha, alot of features are not ready yet and the stdlib only contains core functions**

## Install

```sh
curl -s https://valk-lang.dev/install.sh | bash -s latest
```

## Basic example

```rust
// main.va
fn main() {
    println("Hello Valk! 🎉")
}
```

```sh
valk build main.va -o ./main
./main
```

## Build from source (Linux / macOS / WSL)

macOS: `brew install llvm@15 && brew link llvm@15`

Ubuntu / Debian: `sudo apt-get install llvm-15 clang-15 lld libcurl4-openssl-dev`

```bash
git clone https://github.com/valk-lang/valk.git
cd valk
make
```

## How can it have faster/similar performance as Rust?

Valk is only faster in the way it creates and manages objects, which most programs revolve around. Objects are created using pools. These pools are much faster than using malloc and free all the time (and uses less memory). A GC always has some overhead, but the overall performance gain is much higher than the loss. Which results in Valk being faster than Rust sometimes. Note that our way of doing GC is very different than other languages. Each thread manages its own memory and we only trace when we absolutely have to. 9 out of 10 times we simply clean up the pools instead.

## Benchmarks

<div align="center"><p>
    <img src="https://raw.githubusercontent.com/valk-lang/valk/master/misc/valk-bintree.png">
    <img src="https://raw.githubusercontent.com/valk-lang/valk/master/misc/valk-http.png">
</p></div>

## Good to know

- Each thread handles it's own memory, but you can still share your variables with other threads. Even when your thread has ended, the memory will still be valid and automatically freed once other threads no longer use it.

- Valk does not force the user to use mutexes/locks for shared memory. Therefore the program can crash when you use multiple threads to modify the same data at the same time. Reading the same data with multiple threads is fine.

- Unlike other languages, our GC has no randomness. Every run is exactly the same as the run before to the last byte. So there are no fluctuations in cpu usage and memory usage. (Except when using shared memory over multiple threads)

## Contributions

Once we hit version 0.1.0, we want to look for people who can help with: the standard library, packages and supporting SIMD. If you want to contribute, just hop into the discord and post in general chat or send a private message to the discord owner.

## References

Binary tree benchmark code: [https://programming-language-benchmarks.vercel.app/problem/binarytrees](https://programming-language-benchmarks.vercel.app/problem/binarytrees)
