#include <imx8m/imx8m.hh>

#include <getopt.h>

/**
 * \brief main function
 * 
 * \return int 
 */
int main(int argc, char *argv[]) {
    imx8m::Imx8m::assert_abi();
    if (argc < 2) {
        std::cerr << "Usage: imx8m <CONFIG_FILE>" << std::endl;
        return EXIT_FAILURE;
    }
    imx8m::Imx8m self(argv[1]);

    [[gnu::unlikely]] if (!self.is_ok()) {
        std::cerr << self.get_error_str() << std::endl;
        return EXIT_FAILURE;
    }
    self.run();
    return EXIT_SUCCESS;
}
