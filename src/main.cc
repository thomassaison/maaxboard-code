#include <imx8m/imx8m.hh>

/**
 * \brief main function
 * 
 * \return int 
 */
int main(void) {
    imx8m::Imx8m::assert_abi();

    imx8m::Imx8m self("./example/config/default-config.json");
    return 0;
}
