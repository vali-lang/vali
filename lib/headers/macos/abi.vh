
global errno: i32;
shared stderr : ?ptr;
shared stdin : ?ptr;
shared stdout : ?ptr;

// alias ptr as FILE;
// alias ptr as DIR;

// pid_t = i32
// socklen_t = u32
// mode_t = u32
// uid_t = u32
// gid_t = u32

fn __error() ptr;
fn malloc(size: uint) ptr;
fn free(adr: ptr) void;

fn read(fd: i32, buf: c_string, size: uint) int;
fn write(fd: i32, buf: c_string, size: uint) int;
fn open(path: c_string, flags: i32, mode: i32) i32;
fn close(fd: i32) i32;

fn recv(fd: i32, buf: ptr, len: uint, flags: i32) int;
fn send(fd: i32, buf: ptr, len: uint, flags: i32) int;

fn fcntl(fd: i32, action: i32, value: i32) i32;

// Files
fn stat(path: c_string, stat_buf: cstruct_stat) i32;
fn fstat(fd: i32, stat_buf: cstruct_stat) i32;
fn lstat(path: c_string, stat_buf: cstruct_stat) i32;

fn opendir(name: c_string) ptr;
fn readdir(dirp: ptr) cstruct_dirent;
fn closedir(dirp: ptr) i32;

// OS
fn popen(command: c_string, type: c_string) ?ptr;
fn fgets(s: c_string, n: i32, stream: ptr) ?c_string;
fn pclose(stream: ptr) i32;
fn system(cmd: c_string) i32;
fn nanosleep(req: cstruct_timespec, rem: cstruct_timespec) i32;

// Poll
fn poll(fds: ptr, nfds: u32, timeout: i32) i32;

//fn pipe(pipefd: i32[2]) i32;
//int select(int nfds, fd_set restrict readfds, fd_set restrict writefds, fd_set restrict exceptfds, struct timeval restrict timeout);
fn dup(old_fd: i32) i32;
fn dup2(old_fd: i32, new_fd: i32) i32;

fn socket(domain: i32, type: i32, protocol: i32) i32;
fn connect(sockfd: i32, addr: cstruct_sockaddr, addrlen: u32) i32;
fn accept(sockfd: i32, addr: ?cstruct_sockaddr, addrlen: ?ptr) i32;
//fn accept4(sockfd: i32, addr: ?cstruct_sockaddr, addrlen: ?ptr, flags: i32) i32;
fn shutdown(sockfd: i32, how: i32) i32;
fn bind(sockfd: i32, addr: cstruct_sockaddr, addrlen: u32) i32;
fn listen(sockfd: i32, backlog: i32) i32;

fn getsockopt(sockfd: i32, level: i32, optname: i32, optval: ptr, optlen: u32) i32;
fn setsockopt(sockfd: i32, level: i32, optname: i32, optval: ptr, optlen: u32) i32;
fn getaddrinfo(host: c_string, port: c_string, hints: cstruct_addrinfo, res: ptr) i32;
fn freeaddrinfo(info: cstruct_addrinfo) i32;

//int clone(int (fn)(void *), void stack, int flags, void arg, .../* pid_t parent_tid, void tls, pid_t child_tid */ );
fn fork() i32;
fn vfork() i32;

fn execve(pathname: c_string, argv: ptr, envp: ptr) i32;

//fn wait3(wstatus: i32[1], options: i32, struct rusage rusage) i32;
//fn wait4(pid: i32, wstatus: i32[1], options: i32, struct rusage rusage) i32;

fn kill(pid: i32, sig: i32) i32;
//fn uname(struct utsname buf) i32;

//int fcntl(int fd, int cmd, ... /* arg */ );

fn getcwd(buf: c_string, size: uint) c_string;
//char getwd(char buf);
//char get_current_dir_name();
//int chdir(path: c_string);
//int fchdir(int fd);

fn rename(oldpath: c_string, newpath: c_string) i32;
fn mkdir(pathname: c_string, mode: u32) i32;
fn rmdir(pathname: c_string) i32;
fn link(oldpath: c_string, newpath: c_string) i32;
fn unlink(pathname: c_string) i32;
fn symlink(target: c_string, linkpath: c_string) i32;
fn readlink(pathname: c_string, buf: c_string, bufsiz: uint) int;

fn chmod(pathname: c_string, mode: u32) i32;
fn fchmod(fd: i32, mode: u32) i32;
fn chown(pathname: c_string, owner: u32, group: u32) i32;
fn fchown(fd: i32, owner: u32, group: u32) i32;
fn lchown(pathname: c_string, owner: u32, group: u32) i32;

fn umask(mask: u32) u32;

fn gettimeofday(tv: cstruct_timeval, tz: cstruct_timezone) i32;
fn settimeofday(tv: cstruct_timeval, tz: cstruct_timezone) i32;
//time_t time(time_t tloc);

//int sysinfo(struct sysinfo info);

fn sync() void;

fn gettid() i32;

fn exit(status: i32) void;
fn signal(signum: i32, handler: ?fn(i32)(void)) void;
fn raise(sig: i32) i32;
