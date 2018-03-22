#include "HDF5.h"

#include <cassert>
#include <iostream>

constexpr char FILENAME[] = "abc.h5";

void test_fixed_string() {
  using namespace HDF5;
  File F(FILENAME, "r+");
  {
    std::string s = "abvésdááñere";
    hid_t type_id = H5Tcopy(H5T_C_S1);
    H5Tset_size(type_id, s.size());
    H5Tset_cset(type_id, H5T_CSET_UTF8);

    DataSpace space;

    hid_t attr_id = H5Acreate(F.id, "fixed_str", type_id, space.id, H5P_DEFAULT,
                              H5P_DEFAULT);
    H5Awrite(attr_id, type_id, s.c_str());

    H5Aclose(attr_id);
    H5Tclose(type_id);
  }
  {
    std::string s;
    auto attr = F.open_attribute("fixed_str");
    attr.read(s);
    std::cout << s << std::endl;
  }
}

void test_write() {
  using namespace HDF5;
  File F(FILENAME, H5F_ACC_TRUNC);
  auto g = F.create_group("mygroup");
  auto g2 = g.create_group("abc");
  {
    float data = 3.14;
    g2.write_attribute(data, "myattr");
  }
  {
    DataSpace space_2d{3, 5};
    int N = space_2d.get_select_npoints();
    std::vector<double> x(N);
    for (int n = 0; n < N; ++n) x[n] = 3.2 * n;
    g.write_attribute(x, "attr2d", space_2d);
    g.write_dataset(x, "dset2d", space_2d);
    g.write_dataset(3.2f, "three_two");
    g.write_dataset('a', "char_a");
  }
  {
    std::string s = "aéíñsoj";
    auto dset = g.write_dataset(s, "mystr");
    s += " description";
    dset.write_attribute(s, "description");
  }
  assert(F.get_obj_count() == 3);  // 1 file + 2 groups
}

void test_read() {
  using namespace HDF5;
  File F(FILENAME, "r");
  assert(F.get_obj_count() == 1);  // 1 file
  auto g = F.open_group("mygroup");
  auto g2 = g.open_group("abc");
  {
    // Read scalar attribute into a vector.
    auto data = g2.read_attribute<std::vector<double>>("myattr");
    assert(data.size() == 1);
    std::cout << "myattr = " << data.at(0) << std::endl;  // approx 3.14
  }
  {
    auto attr = g.open_attribute("attr2d");
    std::cout << attr.name() << std::endl;
    std::cout << F.name() << std::endl;
    std::cout << attr.get_file().name() << std::endl;
    std::cout << "\nParent:" << std::endl;
    auto dset = g.open_dataset("dset2d");
    std::cout << dset.name() << std::endl;
    std::cout << dset.parent().name() << std::endl;
    std::cout << dset.parent().parent().name() << std::endl;
  }
  {
    auto x = g.read_attribute<std::vector<double>>("attr2d");
    auto a = g.open_attribute("attr2d");
    auto space = a.get_dataspace();
    dims_t dims = space.size();
    assert(dims.size() == 2);
    if (space.ndims() == 2) {
      const double *p = x.data();
      std::cout << "x =\t[";
      for (size_t i = 0; i < dims[0]; ++i) {
        for (size_t j = 0; j < dims[1]; ++j)
          std::cout << *p++ << "\t";
        std::cout << (i == dims[0] - 1 ? "]\n" : "\n\t ");
      }
    }
  }
  {
    auto dset = g.open_dataset("dset2d");
    auto x = g.read_dataset<std::vector<float>>("dset2d");
    std::cout << "x[2] = " << x.at(2) << std::endl;
  }
  {
    auto dset = g.open_dataset("mystr");
    auto s = dset.read_attribute<std::string>("description");
    std::cout << "description = " << s << std::endl;
  }
  assert(F.get_obj_count() == 3);  // 1 file + 2 groups
}

int main(int argc, char **argv) {
  test_write();
  test_read();
  test_fixed_string();
  return 0;
}
