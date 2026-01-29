/* Adapted from alif/samples/drivers/display/src/main.c
 * Reworked for integration in this demo.
 * Original copyright notice follows:
 * 
 * Copyright (c) 2019 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * Based on ST7789V sample:
 * Copyright (c) 2019 Marc Reilly
 *
 * Copyright 2024 Alif Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/display/cdc200.h>
#ifdef CONFIG_MIPI_DSI
#include <zephyr/drivers/mipi_dsi/dsi_dw.h>
#endif /* CONFIG_MIPI_DSI */

#include <soc_common.h>
#include <se_service.h>

#include <zephyr/drivers/gpio.h>

/* VBAT PWR_CTRL field definitions */
#define VBAT_PWR_CTRL_TX_DPHY_PWR_MASK        (1U << 0) /* Mask off the power supply for MIPI TX DPHY */
#define VBAT_PWR_CTRL_TX_DPHY_ISO             (1U << 1) /* Enable isolation for MIPI TX DPHY */
#define VBAT_PWR_CTRL_RX_DPHY_PWR_MASK        (1U << 4) /* Mask off the power supply for MIPI RX DPHY */
#define VBAT_PWR_CTRL_RX_DPHY_ISO             (1U << 5) /* Enable isolation for MIPI RX DPHY */
#define VBAT_PWR_CTRL_DPHY_PLL_PWR_MASK       (1U << 8) /* Mask off the power supply for MIPI PLL */
#define VBAT_PWR_CTRL_DPHY_PLL_ISO            (1U << 9) /* Enable isolation for MIPI PLL */
#define VBAT_PWR_CTRL_DPHY_VPH_1P8_PWR_BYP_EN (1U << 12) /* dphy vph 1p8 power bypass enable */

#if defined(CONFIG_VIDEO)
#define CAM_PWR_NODE DT_ALIAS(cam_pwr)

#if !DT_NODE_HAS_STATUS(CAM_PWR_NODE, okay)
#error "cam-pwr alias is not defined or not okay"
#endif

static const struct gpio_dt_spec cam_pwr = GPIO_DT_SPEC_GET(CAM_PWR_NODE, gpios);

#define CAM_RESET_NODE DT_ALIAS(cam_reset)

#if !DT_NODE_HAS_STATUS(CAM_RESET_NODE, okay)
#error "cam-reset alias is not defined or not okay"
#endif

static const struct gpio_dt_spec cam_reset = GPIO_DT_SPEC_GET(CAM_RESET_NODE, gpios);


#endif

/**
 * Set the RUN profile parameters for this application.
 */
static int app_set_run_params(void)
{
	run_profile_t runp;
	int ret;

/* Display configuration */
#if (defined(CONFIG_ENSEMBLE_GEN2) && defined(CONFIG_MIPI_DSI))
	const struct gpio_dt_spec cam_disp_mux_gpio =
		GPIO_DT_SPEC_GET(DT_NODELABEL(mipi_dsi), cam_disp_mux_gpios);
	gpio_pin_configure_dt(&cam_disp_mux_gpio, GPIO_OUTPUT_ACTIVE);
#endif

#if (CONFIG_VIDEO_MIPI_CSI2_DW)

#if (DT_NODE_HAS_STATUS(DT_NODELABEL(camera_select), okay))
	const struct gpio_dt_spec sel =
		GPIO_DT_SPEC_GET(DT_NODELABEL(camera_select), select_gpios);

	gpio_pin_configure_dt(&sel, GPIO_OUTPUT);
	gpio_pin_set_dt(&sel, 1);
#endif /* (DT_NODE_HAS_STATUS(DT_NODELABEL(camera_sensor), okay)) */
#endif 


#if (DT_NODE_HAS_STATUS(DT_NODELABEL(lpcam), okay))
	/* Enable LPCAM controller Pixel Clock (XVCLK). */
	/*
	 * Not needed for the time being as LP-CAM supports only
	 * parallel data-mode of cature and only MT9M114 sensor is
	 * tested with parallel data capture which generates clock
	 * internally. But can be used to generate XVCLK from LP CAM
	 * controller.
	 * sys_write32(0x140001, M55HE_CFG_HE_CAMERA_PIXCLK);
	 */
#endif

	/* Enable HFOSC (38.4 MHz) and CFG (100 MHz) clock. */
#if defined(CONFIG_SOC_SERIES_E8)
	sys_set_bits(CGU_CLK_ENA, BIT(23) | BIT(7));
#else
	sys_set_bits(CGU_CLK_ENA, BIT(23) | BIT(21));
#endif /* defined (CONFIG_SOC_SERIES_E7) */

	runp.power_domains = PD_SYST_MASK | PD_SSE700_AON_MASK;
	runp.dcdc_voltage  = 825;
	runp.dcdc_mode     = DCDC_MODE_PWM;
	runp.aon_clk_src   = CLK_SRC_LFXO;
	runp.run_clk_src   = CLK_SRC_PLL;
	runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
#if defined(CONFIG_RTSS_HP)
	runp.cpu_clk_freq  = CLOCK_FREQUENCY_400MHZ;
#else
	runp.cpu_clk_freq  = CLOCK_FREQUENCY_160MHZ;
#endif

	runp.memory_blocks = MRAM_MASK;
#if DT_NODE_EXISTS(DT_NODELABEL(sram0))
	runp.memory_blocks |= SRAM0_MASK;
#endif
#if DT_NODE_EXISTS(DT_NODELABEL(sram1))
	runp.memory_blocks |= SRAM1_MASK;
#endif
#if DT_NODE_EXISTS(DT_NODELABEL(ospi1))
	runp.memory_blocks |= OSPI1_MASK;
#endif
	runp.phy_pwr_gating = MIPI_TX_DPHY_MASK | MIPI_RX_DPHY_MASK |
		MIPI_PLL_DPHY_MASK | LDO_PHY_MASK;
	runp.ip_clock_gating =  OSPI_1_MASK | CAMERA_MASK | CDC200_MASK | MIPI_DSI_MASK | GPU_MASK;

	ret = se_service_set_run_cfg(&runp);
	__ASSERT(ret == 0, "SE: set_run_cfg failed = %d", ret);



	return ret;
}
/*
 * CRITICAL: Must run at PRE_KERNEL_1 to restore SYSTOP before peripherals initialize.
 *
 * Priority 46 ensures this runs:
 *   - AFTER SE Services (priority 45) - SE must be ready for set_run_cfg()
 *   - BEFORE Power Domain (priority 47) - Power domain needs SYSTOP enabled
 *   - BEFORE UART and peripherals (priority 50+) - Peripherals need SYSTOP ON
 *
 * On cold boot: SYSTOP is already ON by default, safe to call.
 * On SOFT_OFF wakeup: SYSTOP is OFF, must restore BEFORE peripherals access registers.
 */
SYS_INIT(app_set_run_params, PRE_KERNEL_1, 46);

static int camera_power(void)
{
	/* Camera configuration */
#if defined(CONFIG_VIDEO)

	gpio_pin_configure_dt(&cam_pwr,GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&cam_reset, GPIO_OUTPUT_INACTIVE);
	
	gpio_pin_set_dt(&cam_pwr, GPIO_OUTPUT_ACTIVE);
	gpio_pin_set_dt(&cam_reset, GPIO_OUTPUT_ACTIVE);
	k_sleep(K_MSEC(5));
	sys_write32(0x140001, M55HE_CFG_HE_CAMERA_PIXCLK);
	gpio_pin_set_dt(&cam_reset, GPIO_OUTPUT_INACTIVE);

	k_sleep(K_MSEC(50));



	sys_clear_bits(VBAT_PWR_CTRL, VBAT_PWR_CTRL_TX_DPHY_PWR_MASK | VBAT_PWR_CTRL_RX_DPHY_PWR_MASK |
                        VBAT_PWR_CTRL_DPHY_PLL_PWR_MASK | VBAT_PWR_CTRL_DPHY_VPH_1P8_PWR_BYP_EN);
	sys_clear_bits(VBAT_PWR_CTRL, VBAT_PWR_CTRL_TX_DPHY_ISO | VBAT_PWR_CTRL_RX_DPHY_ISO | VBAT_PWR_CTRL_DPHY_PLL_ISO);

#endif

	return 0;
}
SYS_INIT(camera_power, POST_KERNEL, 59);
