#include "HDF5.h"

#include <mpi.h>
#include <cassert>
#include <iostream>

constexpr char FILENAME_MPI[] = "abc_parallel.h5";

int myrank(MPI_Comm comm = MPI_COMM_WORLD) {
  int rank;
  MPI_Comm_rank(comm, &rank);
  return rank;
}

int MPI_num_procs(MPI_Comm comm = MPI_COMM_WORLD) {
  int size;
  MPI_Comm_size(comm, &size);
  return size;
}

void test_write_mpi() {
  using namespace HDF5;
  PropList::FileAcc plist;
  const size_t Nproc = MPI_num_procs();
  plist.set_mpio(MPI_COMM_WORLD);
  File ff(FILENAME_MPI, "w", plist);
  {
    // Write 2D array in parallel.
    // Array has dimensions (Nproc, N).
    // The local process writes a single row of the matrix.
    const size_t N = 3;
    const size_t rank = myrank();
    std::vector<int> data(N, 2 * rank);
    DataSpace memspace({1, N});
    DataSpace filespace({Nproc, N});
    auto dset =
        ff.create_dataset("rank_vector", PredType::NATIVE_INT(), filespace);

    PropList::DSetXfer dxpl;
    dxpl.set_mpio_collective();
    // dxpl.set_mpio_independent();

    DataSpace::Hyperslab<2> h;
    h.start = {rank, 0};
    h.count = {1, N};
    filespace.select_hyperslab(h);

    dset.write(data, memspace, filespace, dxpl);
  }
}

void test_read_mpi() {
  using namespace HDF5;
  PropList::FileAcc plist;
  const size_t Nproc = MPI_num_procs();
  plist.set_mpio(MPI_COMM_WORLD);
  File ff(FILENAME_MPI, "r", plist);
  {
    const size_t rank = myrank();
    auto dset = ff.open_dataset("rank_vector");
    auto filespace = dset.get_dataspace();
    const auto dims = filespace.size();
    assert(dims.size() == 2 && dims[0] == Nproc);
    const size_t N = dims[1];
    DataSpace memspace({1, N});

    PropList::DSetXfer dxpl;
    dxpl.set_mpio_collective();

    DataSpace::Hyperslab<2> h;
    h.start = {rank, 0};
    h.count = {1, N};
    filespace.select_hyperslab(h);

    std::vector<int> data;
    dset.read(data, memspace, filespace, dxpl);

    if (rank == Nproc - 1) {
      std::cout << "Reading from parallel file (MPI rank = " << rank << ")\n[";
      for (auto x : data)
        std::cout << x << "\t";
      std::cout << "]\n";
    }
  }
}

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  test_write_mpi();
  test_read_mpi();
  MPI_Finalize();
  return 0;
}
