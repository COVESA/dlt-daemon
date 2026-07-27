class DltDaemon < Formula
  desc "Diagnostic Log and Trace (DLT) library, headers and daemon from COVESA"
  homepage "https://github.com/COVESA/dlt-daemon"
  url "https://github.com/COVESA/dlt-daemon.git", branch: "master"
  version "master"
  license "MPL-2.0"

  head "https://github.com/COVESA/dlt-daemon.git", branch: "master"

  depends_on "cmake" => :build
  depends_on "ninja" => :build
  # Clang is not yet supported on macOS upstream; GCC required (see COVESA/dlt-daemon PR #819)
  depends_on "gcc" => :build

  def install
    gcc = Formula["gcc"]
    gcc_ver = gcc.any_installed_version.major

    args = %W[
      -DCMAKE_C_COMPILER=#{gcc.opt_bin}/gcc-#{gcc_ver}
      -DCMAKE_CXX_COMPILER=#{gcc.opt_bin}/g++-#{gcc_ver}
      -DCMAKE_INSTALL_RPATH=@loader_path/../lib
      -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
      -DCMAKE_BUILD_TYPE=Release
      -DBUILD_SHARED_LIBS=ON
      -DWITH_SYSTEMD=OFF
      -DWITH_SYSTEMD_WATCHDOG=OFF
      -DWITH_SYSTEMD_JOURNAL=OFF
      -DWITH_DLT_CONSOLE=ON
      -DWITH_DLT_CONSOLE_RECEIVE=ON
      -DWITH_DLT_CONSOLE_CONVERT=ON
      -DWITH_DLT_CONSOLE_CONTROL=OFF
      -DWITH_DLT_CONSOLE_PASSIVE_NODE_CTRL=OFF
      -DWITH_DLT_CONSOLE_WO_SBTM=ON
      -DWITH_DLT_ADAPTOR_STDIN=ON
      -DWITH_DLT_EXAMPLES=OFF
      -DWITH_DLT_SYSTEM=OFF
      -DWITH_DLT_DBUS=OFF
      -DWITH_DLT_TESTS=OFF
      -DWITH_DLT_UNIT_TESTS=OFF
      -DBUILD_GMOCK=OFF
      -DWITH_DLT_COVERAGE=OFF
      -DWITH_GIT_SUBMODULE=OFF
      -DWITH_DOC=OFF
      -DWITH_MAN=OFF
      -G Ninja
    ]

    system "cmake", "-S", ".", "-B", "build", *args, *std_cmake_args
    system "cmake", "--build", "build", "--config", "Release"
    system "cmake", "--install", "build"
  end

  test do
    assert_predicate lib/"libdlt.dylib", :exist?
    assert_predicate bin/"dlt-daemon", :exist?
    assert_predicate bin/"dlt-receive", :exist?
    assert_predicate bin/"dlt-convert", :exist?
    assert_predicate bin/"dlt-adaptor-stdin", :exist?
    assert_predicate include/"dlt/dlt.h", :exist?

    (testpath/"test_dlt.c").write <<~C
      #include <dlt/dlt.h>
      int main(void) {
          DLT_DECLARE_CONTEXT(ctx);
          (void)ctx;
          return 0;
      }
    C

    gcc = Formula["gcc"]
    gcc_ver = gcc.any_installed_version.major
    system "#{gcc.opt_bin}/gcc-#{gcc_ver}",
           "-I#{include}", "-L#{lib}", "-ldlt",
           testpath/"test_dlt.c", "-o", testpath/"test_dlt"
  end
end
