// SPDX-License-Identifier: GPL-2.0+
#include <linux/bitfield.h>
#include <linux/bitmap.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/nvmem-consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/phy.h>
#include <linux/regmap.h>
#include <linux/of.h>

#include "../phylib.h"
#include "mtk.h"

#define MTK_GPHY_ID_MT7981			0x03a29461
#define MTK_GPHY_ID_MT7988			0x03a29481

#define MTK_EXT_PAGE_ACCESS			0x1f
#define MTK_PHY_PAGE_STANDARD			0x0000
#define MTK_PHY_PAGE_EXTENDED_3			0x0003

#define MTK_PHY_LPI_REG_14			0x14
#define MTK_PHY_LPI_WAKE_TIMER_1000_MASK	GENMASK(8, 0)

#define MTK_PHY_LPI_REG_1c			0x1c
#define MTK_PHY_SMI_DET_ON_THRESH_MASK		GENMASK(13, 8)

#define MTK_PHY_PAGE_EXTENDED_2A30		0x2a30

/* Registers on Token Ring debug nodes */
/* ch_addr = 0x0, node_addr = 0x7, data_addr = 0x15 */
/* NormMseLoThresh */
#define NORMAL_MSE_LO_THRESH_MASK		GENMASK(15, 8)

/* ch_addr = 0x0, node_addr = 0xf, data_addr = 0x3c */
/* RemAckCntLimitCtrl */
#define REMOTE_ACK_COUNT_LIMIT_CTRL_MASK	GENMASK(2, 1)

/* ch_addr = 0x1, node_addr = 0xd, data_addr = 0x20 */
/* VcoSlicerThreshBitsHigh */
#define VCO_SLICER_THRESH_HIGH_MASK		GENMASK(23, 0)

/* ch_addr = 0x1, node_addr = 0xf, data_addr = 0x0 */
/* DfeTailEnableVgaThresh1000 */
#define DFE_TAIL_EANBLE_VGA_TRHESH_1000		GENMASK(5, 1)

/* ch_addr = 0x1, node_addr = 0xf, data_addr = 0x1 */
/* MrvlTrFix100Kp */
#define MRVL_TR_FIX_100KP_MASK			GENMASK(22, 20)
/* MrvlTrFix100Kf */
#define MRVL_TR_FIX_100KF_MASK			GENMASK(19, 17)
/* MrvlTrFix1000Kp */
#define MRVL_TR_FIX_1000KP_MASK			GENMASK(16, 14)
/* MrvlTrFix1000Kf */
#define MRVL_TR_FIX_1000KF_MASK			GENMASK(13, 11)

/* ch_addr = 0x1, node_addr = 0xf, data_addr = 0x12 */
/* VgaDecRate */
#define VGA_DECIMATION_RATE_MASK		GENMASK(8, 5)

/* ch_addr = 0x1, node_addr = 0xf, data_addr = 0x17 */
/* SlvDSPreadyTime */
#define SLAVE_DSP_READY_TIME_MASK		GENMASK(22, 15)
/* MasDSPreadyTime */
#define MASTER_DSP_READY_TIME_MASK		GENMASK(14, 7)

/* ch_addr = 0x1, node_addr = 0xf, data_addr = 0x18 */
/* EnabRandUpdTrig */
#define ENABLE_RANDOM_UPDOWN_COUNTER_TRIGGER	BIT(8)

/* ch_addr = 0x1, node_addr = 0xf, data_addr = 0x20 */
/* ResetSyncOffset */
#define RESET_SYNC_OFFSET_MASK			GENMASK(11, 8)

/* ch_addr = 0x2, node_addr = 0xd, data_addr = 0x0 */
/* FfeUpdGainForceVal */
#define FFE_UPDATE_GAIN_FORCE_VAL_MASK		GENMASK(9, 7)
/* FfeUpdGainForce */
#define FFE_UPDATE_GAIN_FORCE			BIT(6)

/* ch_addr = 0x2, node_addr = 0xd, data_addr = 0x3 */
/* TrFreeze */
#define TR_FREEZE_MASK				GENMASK(11, 0)

/* ch_addr = 0x2, node_addr = 0xd, data_addr = 0x6 */
/* SS: Steady-state, KP: Proportional Gain */
/* SSTrKp100 */
#define SS_TR_KP100_MASK			GENMASK(21, 19)
/* SSTrKf100 */
#define SS_TR_KF100_MASK			GENMASK(18, 16)
/* SSTrKp1000Mas */
#define SS_TR_KP1000_MASTER_MASK		GENMASK(15, 13)
/* SSTrKf1000Mas */
#define SS_TR_KF1000_MASTER_MASK		GENMASK(12, 10)
/* SSTrKp1000Slv */
#define SS_TR_KP1000_SLAVE_MASK			GENMASK(9, 7)
/* SSTrKf1000Slv */
#define SS_TR_KF1000_SLAVE_MASK			GENMASK(6, 4)

/* ch_addr = 0x2, node_addr = 0xd, data_addr = 0x8 */
/* clear this bit if wanna select from AFE */
/* Regsigdet_sel_1000 */
#define EEE1000_SELECT_SIGNAL_DETECTION_FROM_DFE	BIT(4)

/* ch_addr = 0x2, node_addr = 0xd, data_addr = 0xd */
/* RegEEE_st2TrKf1000 */
#define EEE1000_STAGE2_TR_KF_MASK		GENMASK(13, 11)

/* ch_addr = 0x2, node_addr = 0xd, data_addr = 0xf */
/* RegEEE_slv_waketr_timer_tar */
#define SLAVE_WAKETR_TIMER_MASK			GENMASK(20, 11)
/* RegEEE_slv_remtx_timer_tar */
#define SLAVE_REMTX_TIMER_MASK			GENMASK(10, 1)

/* ch_addr = 0x2, node_addr = 0xd, data_addr = 0x10 */
/* RegEEE_slv_wake_int_timer_tar */
#define SLAVE_WAKEINT_TIMER_MASK		GENMASK(10, 1)

/* ch_addr = 0x2, node_addr = 0xd, data_addr = 0x14 */
/* RegEEE_trfreeze_timer2 */
#define TR_FREEZE_TIMER2_MASK			GENMASK(9, 0)

/* ch_addr = 0x2, node_addr = 0xd, data_addr = 0x1c */
/* RegEEE100Stg1_tar */
#define EEE100_LPSYNC_STAGE1_UPDATE_TIMER_MASK	GENMASK(8, 0)

/* ch_addr = 0x2, node_addr = 0xd, data_addr = 0x25 */
/* REGEEE_wake_slv_tr_wait_dfesigdet_en */
#define WAKE_SLAVE_TR_WAIT_DFE_DETECTION_EN	BIT(11)

#define ANALOG_INTERNAL_OPERATION_MAX_US	20
#define TXRESERVE_MIN				0
#define TXRESERVE_MAX				7

#define MTK_PHY_ANARG_RG			0x10
#define   MTK_PHY_TCLKOFFSET_MASK		GENMASK(12, 8)

/* Registers on MDIO_MMD_VEND1 */
#define MTK_PHY_TXVLD_DA_RG			0x12
#define   MTK_PHY_DA_TX_I2MPB_A_GBE_MASK	GENMASK(15, 10)
#define   MTK_PHY_DA_TX_I2MPB_A_TBT_MASK	GENMASK(5, 0)

#define MTK_PHY_TX_I2MPB_TEST_MODE_A2		0x16
#define   MTK_PHY_DA_TX_I2MPB_A_HBT_MASK	GENMASK(15, 10)
#define   MTK_PHY_DA_TX_I2MPB_A_TST_MASK	GENMASK(5, 0)

#define MTK_PHY_TX_I2MPB_TEST_MODE_B1		0x17
#define   MTK_PHY_DA_TX_I2MPB_B_GBE_MASK	GENMASK(13, 8)
#define   MTK_PHY_DA_TX_I2MPB_B_TBT_MASK	GENMASK(5, 0)

#define MTK_PHY_TX_I2MPB_TEST_MODE_B2		0x18
#define   MTK_PHY_DA_TX_I2MPB_B_HBT_MASK	GENMASK(13, 8)
#define   MTK_PHY_DA_TX_I2MPB_B_TST_MASK	GENMASK(5, 0)

#define MTK_PHY_TX_I2MPB_TEST_MODE_C1		0x19
#define   MTK_PHY_DA_TX_I2MPB_C_GBE_MASK	GENMASK(13, 8)
#define   MTK_PHY_DA_TX_I2MPB_C_TBT_MASK	GENMASK(5, 0)

#define MTK_PHY_TX_I2MPB_TEST_MODE_C2		0x20
#define   MTK_PHY_DA_TX_I2MPB_C_HBT_MASK	GENMASK(13, 8)
#define   MTK_PHY_DA_TX_I2MPB_C_TST_MASK	GENMASK(5, 0)

#define MTK_PHY_TX_I2MPB_TEST_MODE_D1		0x21
#define   MTK_PHY_DA_TX_I2MPB_D_GBE_MASK	GENMASK(13, 8)
#define   MTK_PHY_DA_TX_I2MPB_D_TBT_MASK	GENMASK(5, 0)

#define MTK_PHY_TX_I2MPB_TEST_MODE_D2		0x22
#define   MTK_PHY_DA_TX_I2MPB_D_HBT_MASK	GENMASK(13, 8)
#define   MTK_PHY_DA_TX_I2MPB_D_TST_MASK	GENMASK(5, 0)

#define MTK_PHY_RXADC_CTRL_RG7			0xc6
#define   MTK_PHY_DA_AD_BUF_BIAS_LP_MASK	GENMASK(9, 8)

#define MTK_PHY_RXADC_CTRL_RG9			0xc8
#define   MTK_PHY_DA_RX_PSBN_TBT_MASK		GENMASK(14, 12)
#define   MTK_PHY_DA_RX_PSBN_HBT_MASK		GENMASK(10, 8)
#define   MTK_PHY_DA_RX_PSBN_GBE_MASK		GENMASK(6, 4)
#define   MTK_PHY_DA_RX_PSBN_LP_MASK		GENMASK(2, 0)

#define MTK_PHY_LDO_OUTPUT_V			0xd7

#define MTK_PHY_RG_ANA_CAL_RG0			0xdb
#define   MTK_PHY_RG_CAL_CKINV			BIT(12)
#define   MTK_PHY_RG_ANA_CALEN			BIT(8)
#define   MTK_PHY_RG_ZCALEN_A			BIT(0)

#define MTK_PHY_RG_ANA_CAL_RG1			0xdc
#define   MTK_PHY_RG_ZCALEN_B			BIT(12)
#define   MTK_PHY_RG_ZCALEN_C			BIT(8)
#define   MTK_PHY_RG_ZCALEN_D			BIT(4)
#define   MTK_PHY_RG_TXVOS_CALEN		BIT(0)

#define MTK_PHY_RG_ANA_CAL_RG5			0xe0
#define   MTK_PHY_RG_REXT_TRIM_MASK		GENMASK(13, 8)

#define MTK_PHY_RG_TX_FILTER			0xfe

#define MTK_PHY_RG_LPI_PCS_DSP_CTRL_REG120	0x120
#define   MTK_PHY_LPI_SIG_EN_LO_THRESH1000_MASK	GENMASK(12, 8)
#define   MTK_PHY_LPI_SIG_EN_HI_THRESH1000_MASK	GENMASK(4, 0)

#define MTK_PHY_RG_LPI_PCS_DSP_CTRL_REG122	0x122
#define   MTK_PHY_LPI_NORM_MSE_HI_THRESH1000_MASK	GENMASK(7, 0)

#define MTK_PHY_RG_TESTMUX_ADC_CTRL		0x144
#define   MTK_PHY_RG_TXEN_DIG_MASK		GENMASK(5, 5)

#define MTK_PHY_RG_CR_TX_AMP_OFFSET_A_B		0x172
#define   MTK_PHY_CR_TX_AMP_OFFSET_A_MASK	GENMASK(13, 8)
#define   MTK_PHY_CR_TX_AMP_OFFSET_B_MASK	GENMASK(6, 0)

#define MTK_PHY_RG_CR_TX_AMP_OFFSET_C_D		0x173
#define   MTK_PHY_CR_TX_AMP_OFFSET_C_MASK	GENMASK(13, 8)
#define   MTK_PHY_CR_TX_AMP_OFFSET_D_MASK	GENMASK(6, 0)

#define MTK_PHY_RG_AD_CAL_COMP			0x17a
#define   MTK_PHY_AD_CAL_COMP_OUT_MASK		GENMASK(8, 8)

#define MTK_PHY_RG_AD_CAL_CLK			0x17b
#define   MTK_PHY_DA_CAL_CLK			BIT(0)

#define MTK_PHY_RG_AD_CALIN			0x17c
#define   MTK_PHY_DA_CALIN_FLAG			BIT(0)

#define MTK_PHY_RG_DASN_DAC_IN0_A		0x17d
#define   MTK_PHY_DASN_DAC_IN0_A_MASK		GENMASK(9, 0)

#define MTK_PHY_RG_DASN_DAC_IN0_B		0x17e
#define   MTK_PHY_DASN_DAC_IN0_B_MASK		GENMASK(9, 0)

#define MTK_PHY_RG_DASN_DAC_IN0_C		0x17f
#define   MTK_PHY_DASN_DAC_IN0_C_MASK		GENMASK(9, 0)

#define MTK_PHY_RG_DASN_DAC_IN0_D		0x180
#define   MTK_PHY_DASN_DAC_IN0_D_MASK		GENMASK(9, 0)

#define MTK_PHY_RG_DASN_DAC_IN1_A		0x181
#define   MTK_PHY_DASN_DAC_IN1_A_MASK		GENMASK(9, 0)

#define MTK_PHY_RG_DASN_DAC_IN1_B		0x182
#define   MTK_PHY_DASN_DAC_IN1_B_MASK		GENMASK(9, 0)

#define MTK_PHY_RG_DASN_DAC_IN1_C		0x183
#define   MTK_PHY_DASN_DAC_IN1_C_MASK		GENMASK(9, 0)

#define MTK_PHY_RG_DASN_DAC_IN1_D		0x184
#define   MTK_PHY_DASN_DAC_IN1_D_MASK		GENMASK(9, 0)

#define MTK_PHY_RG_DEV1E_REG19b			0x19b
#define   MTK_PHY_BYPASS_DSP_LPI_READY		BIT(8)

#define MTK_PHY_RG_LP_IIR2_K1_L			0x22a
#define MTK_PHY_RG_LP_IIR2_K1_U			0x22b
#define MTK_PHY_RG_LP_IIR2_K2_L			0x22c
#define MTK_PHY_RG_LP_IIR2_K2_U			0x22d
#define MTK_PHY_RG_LP_IIR2_K3_L			0x22e
#define MTK_PHY_RG_LP_IIR2_K3_U			0x22f
#define MTK_PHY_RG_LP_IIR2_K4_L			0x230
#define MTK_PHY_RG_LP_IIR2_K4_U			0x231
#define MTK_PHY_RG_LP_IIR2_K5_L			0x232
#define MTK_PHY_RG_LP_IIR2_K5_U			0x233

#define MTK_PHY_RG_DEV1E_REG234			0x234
#define   MTK_PHY_TR_OPEN_LOOP_EN_MASK		GENMASK(0, 0)
#define   MTK_PHY_LPF_X_AVERAGE_MASK		GENMASK(7, 4)
#define   MTK_PHY_TR_LP_IIR_EEE_EN		BIT(12)

#define MTK_PHY_RG_LPF_CNT_VAL			0x235

#define MTK_PHY_RG_DEV1E_REG238			0x238
#define   MTK_PHY_LPI_SLV_SEND_TX_TIMER_MASK	GENMASK(8, 0)
#define   MTK_PHY_LPI_SLV_SEND_TX_EN		BIT(12)

#define MTK_PHY_RG_DEV1E_REG239			0x239
#define   MTK_PHY_LPI_SEND_LOC_TIMER_MASK	GENMASK(8, 0)
#define   MTK_PHY_LPI_TXPCS_LOC_RCV		BIT(12)

#define MTK_PHY_RG_DEV1E_REG27C			0x27c
#define   MTK_PHY_VGASTATE_FFE_THR_ST1_MASK	GENMASK(12, 8)
#define MTK_PHY_RG_DEV1E_REG27D			0x27d
#define   MTK_PHY_VGASTATE_FFE_THR_ST2_MASK	GENMASK(4, 0)

#define MTK_PHY_RG_DEV1E_REG2C7			0x2c7
#define   MTK_PHY_MAX_GAIN_MASK			GENMASK(4, 0)
#define   MTK_PHY_MIN_GAIN_MASK			GENMASK(12, 8)

#define MTK_PHY_RG_DEV1E_REG2D1			0x2d1
#define   MTK_PHY_VCO_SLICER_THRESH_BITS_HIGH_EEE_MASK	GENMASK(7, 0)
#define   MTK_PHY_LPI_SKIP_SD_SLV_TR		BIT(8)
#define   MTK_PHY_LPI_TR_READY			BIT(9)
#define   MTK_PHY_LPI_VCO_EEE_STG0_EN		BIT(10)

#define MTK_PHY_RG_DEV1E_REG323			0x323
#define   MTK_PHY_EEE_WAKE_MAS_INT_DC		BIT(0)
#define   MTK_PHY_EEE_WAKE_SLV_INT_DC		BIT(4)

#define MTK_PHY_RG_DEV1E_REG324			0x324
#define   MTK_PHY_SMI_DETCNT_MAX_MASK		GENMASK(5, 0)
#define   MTK_PHY_SMI_DET_MAX_EN		BIT(8)

#define MTK_PHY_RG_DEV1E_REG326			0x326
#define   MTK_PHY_LPI_MODE_SD_ON		BIT(0)
#define   MTK_PHY_RESET_RANDUPD_CNT		BIT(1)
#define   MTK_PHY_TREC_UPDATE_ENAB_CLR		BIT(2)
#define   MTK_PHY_LPI_QUIT_WAIT_DFE_SIG_DET_OFF	BIT(4)
#define   MTK_PHY_TR_READY_SKIP_AFE_WAKEUP	BIT(5)

#define MTK_PHY_LDO_PUMP_EN_PAIRAB		0x502
#define MTK_PHY_LDO_PUMP_EN_PAIRCD		0x503

#define MTK_PHY_DA_TX_R50_PAIR_A		0x53d
#define MTK_PHY_DA_TX_R50_PAIR_B		0x53e
#define MTK_PHY_DA_TX_R50_PAIR_C		0x53f
#define MTK_PHY_DA_TX_R50_PAIR_D		0x540

/* Registers on MDIO_MMD_VEND2 */
#define MTK_PHY_LED1_DEFAULT_POLARITIES		BIT(1)

#define MTK_PHY_RG_BG_RASEL			0x115
#define   MTK_PHY_RG_BG_RASEL_MASK		GENMASK(2, 0)

/* 'boottrap' register reflecting the configuration of the 4 PHY LEDs */
#define RG_GPIO_MISC_TPBANK0			0x6f0
#define   RG_GPIO_MISC_TPBANK0_BOOTMODE		GENMASK(11, 8)

/* These macro privides efuse parsing for internal phy. */
#define EFS_DA_TX_I2MPB_A(x)			(((x) >> 0) & GENMASK(5, 0))
#define EFS_DA_TX_I2MPB_B(x)			(((x) >> 6) & GENMASK(5, 0))
#define EFS_DA_TX_I2MPB_C(x)			(((x) >> 12) & GENMASK(5, 0))
#define EFS_DA_TX_I2MPB_D(x)			(((x) >> 18) & GENMASK(5, 0))
#define EFS_DA_TX_AMP_OFFSET_A(x)		(((x) >> 24) & GENMASK(5, 0))

#define EFS_DA_TX_AMP_OFFSET_B(x)		(((x) >> 0) & GENMASK(5, 0))
#define EFS_DA_TX_AMP_OFFSET_C(x)		(((x) >> 6) & GENMASK(5, 0))
#define EFS_DA_TX_AMP_OFFSET_D(x)		(((x) >> 12) & GENMASK(5, 0))
#define EFS_DA_TX_R50_A(x)			(((x) >> 18) & GENMASK(5, 0))
#define EFS_DA_TX_R50_B(x)			(((x) >> 24) & GENMASK(5, 0))

#define EFS_DA_TX_R50_C(x)			(((x) >> 0) & GENMASK(5, 0))
#define EFS_DA_TX_R50_D(x)			(((x) >> 6) & GENMASK(5, 0))

#define EFS_RG_BG_RASEL(x)			(((x) >> 4) & GENMASK(2, 0))
#define EFS_RG_REXT_TRIM(x)			(((x) >> 7) & GENMASK(5, 0))

enum {
	NO_PAIR,
	PAIR_A,
	PAIR_B,
	PAIR_C,
	PAIR_D,
};

enum calibration_mode {
	EFUSE_K,
	SW_K
};

enum CAL_ITEM {
	REXT,
	TX_OFFSET,
	TX_AMP,
	TX_R50,
	TX_VCM
};

enum CAL_MODE {
	EFUSE_M,
	SW_M
};

struct mtk_socphy_shared {
	u32			boottrap;
	struct mtk_socphy_priv	priv[4];
};

/* One calibration cycle consists of:
 * 1.Set DA_CALIN_FLAG high to start calibration. Keep it high
 *   until AD_CAL_COMP is ready to output calibration result.
 * 2.Wait until DA_CAL_CLK is available.
 * 3.Fetch AD_CAL_COMP_OUT.
 */
static int cal_cycle(struct phy_device *phydev, int devad,
		     u32 regnum, u16 mask, u16 cal_val)
{
	int reg_val;
	int ret;

	phy_modify_mmd(phydev, devad, regnum,
		       mask, cal_val);
	phy_set_bits_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_AD_CALIN,
			 MTK_PHY_DA_CALIN_FLAG);

	ret = phy_read_mmd_poll_timeout(phydev, MDIO_MMD_VEND1,
					MTK_PHY_RG_AD_CAL_CLK, reg_val,
					reg_val & MTK_PHY_DA_CAL_CLK, 500,
					ANALOG_INTERNAL_OPERATION_MAX_US,
					false);
	if (ret) {
		phydev_err(phydev, "Calibration cycle timeout\n");
		return ret;
	}

	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_AD_CALIN,
			   MTK_PHY_DA_CALIN_FLAG);
	ret = phy_read_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_AD_CAL_COMP);
	if (ret < 0)
		return ret;
	ret = FIELD_GET(MTK_PHY_AD_CAL_COMP_OUT_MASK, ret);
	phydev_dbg(phydev, "cal_val: 0x%x, ret: %d\n", cal_val, ret);

	return ret;
}

static int rext_fill_result(struct phy_device *phydev, u16 *buf)
{
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG5,
		       MTK_PHY_RG_REXT_TRIM_MASK, buf[0] << 8);
	phy_modify_mmd(phydev, MDIO_MMD_VEND2, MTK_PHY_RG_BG_RASEL,
		       MTK_PHY_RG_BG_RASEL_MASK, buf[1]);

	return 0;
}

static int rext_cal_efuse(struct phy_device *phydev, u32 *buf)
{
	u16 rext_cal_val[2];

	rext_cal_val[0] = EFS_RG_REXT_TRIM(buf[3]);
	rext_cal_val[1] = EFS_RG_BG_RASEL(buf[3]);
	rext_fill_result(phydev, rext_cal_val);

	return 0;
}

static int tx_offset_fill_result(struct phy_device *phydev, u16 *buf)
{
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_CR_TX_AMP_OFFSET_A_B,
		       MTK_PHY_CR_TX_AMP_OFFSET_A_MASK, buf[0] << 8);
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_CR_TX_AMP_OFFSET_A_B,
		       MTK_PHY_CR_TX_AMP_OFFSET_B_MASK, buf[1]);
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_CR_TX_AMP_OFFSET_C_D,
		       MTK_PHY_CR_TX_AMP_OFFSET_C_MASK, buf[2] << 8);
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_CR_TX_AMP_OFFSET_C_D,
		       MTK_PHY_CR_TX_AMP_OFFSET_D_MASK, buf[3]);

	return 0;
}

static int tx_offset_cal_efuse(struct phy_device *phydev, u32 *buf)
{
	u16 tx_offset_cal_val[4];

	tx_offset_cal_val[0] = EFS_DA_TX_AMP_OFFSET_A(buf[0]);
	tx_offset_cal_val[1] = EFS_DA_TX_AMP_OFFSET_B(buf[1]);
	tx_offset_cal_val[2] = EFS_DA_TX_AMP_OFFSET_C(buf[1]);
	tx_offset_cal_val[3] = EFS_DA_TX_AMP_OFFSET_D(buf[1]);

	tx_offset_fill_result(phydev, tx_offset_cal_val);

	return 0;
}

static int tx_amp_fill_result(struct phy_device *phydev, u16 *buf)
{
	const int vals_9481[16] = { 10, 6, 6, 10,
				    10, 6, 6, 10,
				    10, 6, 6, 10,
				    10, 6, 6, 10 };
	const int vals_9461[16] = { 7, 1, 4, 7,
				    7, 1, 4, 7,
				    7, 1, 4, 7,
				    7, 1, 4, 7 };
	int bias[16] = {};
	int i;

	switch (phydev->drv->phy_id) {
	case MTK_GPHY_ID_MT7981:
		/* We add some calibration to efuse values
		 * due to board level influence.
		 * GBE: +7, TBT: +1, HBT: +4, TST: +7
		 */
		memcpy(bias, (const void *)vals_9461, sizeof(bias));
		break;
	case MTK_GPHY_ID_MT7988:
		memcpy(bias, (const void *)vals_9481, sizeof(bias));
		break;
	}

	/* Prevent overflow */
	for (i = 0; i < 12; i++) {
		if (buf[i >> 2] + bias[i] > 63) {
			buf[i >> 2] = 63;
			bias[i] = 0;
		}
	}

	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_TXVLD_DA_RG,
		       MTK_PHY_DA_TX_I2MPB_A_GBE_MASK,
		       FIELD_PREP(MTK_PHY_DA_TX_I2MPB_A_GBE_MASK,
				  buf[0] + bias[0]));
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_TXVLD_DA_RG,
		       MTK_PHY_DA_TX_I2MPB_A_TBT_MASK,
		       FIELD_PREP(MTK_PHY_DA_TX_I2MPB_A_TBT_MASK,
				  buf[0] + bias[1]));
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_TX_I2MPB_TEST_MODE_A2,
		       MTK_PHY_DA_TX_I2MPB_A_HBT_MASK,
		       FIELD_PREP(MTK_PHY_DA_TX_I2MPB_A_HBT_MASK,
				  buf[0] + bias[2]));
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_TX_I2MPB_TEST_MODE_A2,
		       MTK_PHY_DA_TX_I2MPB_A_TST_MASK,
		       FIELD_PREP(MTK_PHY_DA_TX_I2MPB_A_TST_MASK,
				  buf[0] + bias[3]));

	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_TX_I2MPB_TEST_MODE_B1,
		       MTK_PHY_DA_TX_I2MPB_B_GBE_MASK,
		       FIELD_PREP(MTK_PHY_DA_TX_I2MPB_B_GBE_MASK,
				  buf[1] + bias[4]));
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_TX_I2MPB_TEST_MODE_B1,
		       MTK_PHY_DA_TX_I2MPB_B_TBT_MASK,
		       FIELD_PREP(MTK_PHY_DA_TX_I2MPB_B_TBT_MASK,
				  buf[1] + bias[5]));
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_TX_I2MPB_TEST_MODE_B2,
		       MTK_PHY_DA_TX_I2MPB_B_HBT_MASK,
		       FIELD_PREP(MTK_PHY_DA_TX_I2MPB_B_HBT_MASK,
				  buf[1] + bias[6]));
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_TX_I2MPB_TEST_MODE_B2,
		       MTK_PHY_DA_TX_I2MPB_B_TST_MASK,
		       FIELD_PREP(MTK_PHY_DA_TX_I2MPB_B_TST_MASK,
				  buf[1] + bias[7]));

	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_TX_I2MPB_TEST_MODE_C1,
		       MTK_PHY_DA_TX_I2MPB_C_GBE_MASK,
		       FIELD_PREP(MTK_PHY_DA_TX_I2MPB_C_GBE_MASK,
				  buf[2] + bias[8]));
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_TX_I2MPB_TEST_MODE_C1,
		       MTK_PHY_DA_TX_I2MPB_C_TBT_MASK,
		       FIELD_PREP(MTK_PHY_DA_TX_I2MPB_C_TBT_MASK,
				  buf[2] + bias[9]));
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_TX_I2MPB_TEST_MODE_C2,
		       MTK_PHY_DA_TX_I2MPB_C_HBT_MASK,
		       FIELD_PREP(MTK_PHY_DA_TX_I2MPB_C_HBT_MASK,
				  buf[2] + bias[10]));
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_TX_I2MPB_TEST_MODE_C2,
		       MTK_PHY_DA_TX_I2MPB_C_TST_MASK,
		       FIELD_PREP(MTK_PHY_DA_TX_I2MPB_C_TST_MASK,
				  buf[2] + bias[11]));

	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_TX_I2MPB_TEST_MODE_D1,
		       MTK_PHY_DA_TX_I2MPB_D_GBE_MASK,
		       FIELD_PREP(MTK_PHY_DA_TX_I2MPB_D_GBE_MASK,
				  buf[3] + bias[12]));
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_TX_I2MPB_TEST_MODE_D1,
		       MTK_PHY_DA_TX_I2MPB_D_TBT_MASK,
		       FIELD_PREP(MTK_PHY_DA_TX_I2MPB_D_TBT_MASK,
				  buf[3] + bias[13]));
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_TX_I2MPB_TEST_MODE_D2,
		       MTK_PHY_DA_TX_I2MPB_D_HBT_MASK,
		       FIELD_PREP(MTK_PHY_DA_TX_I2MPB_D_HBT_MASK,
				  buf[3] + bias[14]));
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_TX_I2MPB_TEST_MODE_D2,
		       MTK_PHY_DA_TX_I2MPB_D_TST_MASK,
		       FIELD_PREP(MTK_PHY_DA_TX_I2MPB_D_TST_MASK,
				  buf[3] + bias[15]));

	return 0;
}

static int tx_amp_cal_efuse(struct phy_device *phydev, u32 *buf)
{
	u16 tx_amp_cal_val[4];

	tx_amp_cal_val[0] = EFS_DA_TX_I2MPB_A(buf[0]);
	tx_amp_cal_val[1] = EFS_DA_TX_I2MPB_B(buf[0]);
	tx_amp_cal_val[2] = EFS_DA_TX_I2MPB_C(buf[0]);
	tx_amp_cal_val[3] = EFS_DA_TX_I2MPB_D(buf[0]);
	tx_amp_fill_result(phydev, tx_amp_cal_val);

	return 0;
}

static int tx_r50_fill_result(struct phy_device *phydev, u16 tx_r50_cal_val,
			      u8 txg_calen_x)
{
	int bias = 0;
	u16 reg, val;

	if (phydev->drv->phy_id == MTK_GPHY_ID_MT7988)
		bias = -1;

	val = clamp_val(bias + tx_r50_cal_val, 0, 63);

	switch (txg_calen_x) {
	case PAIR_A:
		reg = MTK_PHY_DA_TX_R50_PAIR_A;
		break;
	case PAIR_B:
		reg = MTK_PHY_DA_TX_R50_PAIR_B;
		break;
	case PAIR_C:
		reg = MTK_PHY_DA_TX_R50_PAIR_C;
		break;
	case PAIR_D:
		reg = MTK_PHY_DA_TX_R50_PAIR_D;
		break;
	default:
		return -EINVAL;
	}

	phy_write_mmd(phydev, MDIO_MMD_VEND1, reg, val | val << 8);

	return 0;
}

static int tx_r50_cal_efuse(struct phy_device *phydev, u32 *buf,
			    u8 txg_calen_x)
{
	u16 tx_r50_cal_val;

	switch (txg_calen_x) {
	case PAIR_A:
		tx_r50_cal_val = EFS_DA_TX_R50_A(buf[1]);
		break;
	case PAIR_B:
		tx_r50_cal_val = EFS_DA_TX_R50_B(buf[1]);
		break;
	case PAIR_C:
		tx_r50_cal_val = EFS_DA_TX_R50_C(buf[2]);
		break;
	case PAIR_D:
		tx_r50_cal_val = EFS_DA_TX_R50_D(buf[2]);
		break;
	default:
		return -EINVAL;
	}
	tx_r50_fill_result(phydev, tx_r50_cal_val, txg_calen_x);

	return 0;
}

static int tx_vcm_cal_sw(struct phy_device *phydev, u8 rg_txreserve_x)
{
	u8 lower_idx, upper_idx, txreserve_val;
	u8 lower_ret, upper_ret;
	int ret;

	phy_set_bits_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0,
			 MTK_PHY_RG_ANA_CALEN);
	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0,
			   MTK_PHY_RG_CAL_CKINV);
	phy_set_bits_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG1,
			 MTK_PHY_RG_TXVOS_CALEN);

	switch (rg_txreserve_x) {
	case PAIR_A:
		phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1,
				   MTK_PHY_RG_DASN_DAC_IN0_A,
				   MTK_PHY_DASN_DAC_IN0_A_MASK);
		phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1,
				   MTK_PHY_RG_DASN_DAC_IN1_A,
				   MTK_PHY_DASN_DAC_IN1_A_MASK);
		phy_set_bits_mmd(phydev, MDIO_MMD_VEND1,
				 MTK_PHY_RG_ANA_CAL_RG0,
				 MTK_PHY_RG_ZCALEN_A);
		break;
	case PAIR_B:
		phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1,
				   MTK_PHY_RG_DASN_DAC_IN0_B,
				   MTK_PHY_DASN_DAC_IN0_B_MASK);
		phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1,
				   MTK_PHY_RG_DASN_DAC_IN1_B,
				   MTK_PHY_DASN_DAC_IN1_B_MASK);
		phy_set_bits_mmd(phydev, MDIO_MMD_VEND1,
				 MTK_PHY_RG_ANA_CAL_RG1,
				 MTK_PHY_RG_ZCALEN_B);
		break;
	case PAIR_C:
		phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1,
				   MTK_PHY_RG_DASN_DAC_IN0_C,
				   MTK_PHY_DASN_DAC_IN0_C_MASK);
		phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1,
				   MTK_PHY_RG_DASN_DAC_IN1_C,
				   MTK_PHY_DASN_DAC_IN1_C_MASK);
		phy_set_bits_mmd(phydev, MDIO_MMD_VEND1,
				 MTK_PHY_RG_ANA_CAL_RG1,
				 MTK_PHY_RG_ZCALEN_C);
		break;
	case PAIR_D:
		phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1,
				   MTK_PHY_RG_DASN_DAC_IN0_D,
				   MTK_PHY_DASN_DAC_IN0_D_MASK);
		phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1,
				   MTK_PHY_RG_DASN_DAC_IN1_D,
				   MTK_PHY_DASN_DAC_IN1_D_MASK);
		phy_set_bits_mmd(phydev, MDIO_MMD_VEND1,
				 MTK_PHY_RG_ANA_CAL_RG1,
				 MTK_PHY_RG_ZCALEN_D);
		break;
	default:
		ret = -EINVAL;
		goto restore;
	}

	lower_idx = TXRESERVE_MIN;
	upper_idx = TXRESERVE_MAX;

	phydev_dbg(phydev, "Start TX-VCM SW cal.\n");
	while ((upper_idx - lower_idx) > 1) {
		txreserve_val = DIV_ROUND_CLOSEST(lower_idx + upper_idx, 2);
		ret = cal_cycle(phydev, MDIO_MMD_VEND1, MTK_PHY_RXADC_CTRL_RG9,
				MTK_PHY_DA_RX_PSBN_TBT_MASK |
				MTK_PHY_DA_RX_PSBN_HBT_MASK |
				MTK_PHY_DA_RX_PSBN_GBE_MASK |
				MTK_PHY_DA_RX_PSBN_LP_MASK,
				txreserve_val << 12 | txreserve_val << 8 |
				txreserve_val << 4 | txreserve_val);
		if (ret == 1) {
			upper_idx = txreserve_val;
			upper_ret = ret;
		} else if (ret == 0) {
			lower_idx = txreserve_val;
			lower_ret = ret;
		} else {
			goto restore;
		}
	}

	if (lower_idx == TXRESERVE_MIN) {
		lower_ret = cal_cycle(phydev, MDIO_MMD_VEND1,
				      MTK_PHY_RXADC_CTRL_RG9,
				      MTK_PHY_DA_RX_PSBN_TBT_MASK |
				      MTK_PHY_DA_RX_PSBN_HBT_MASK |
				      MTK_PHY_DA_RX_PSBN_GBE_MASK |
				      MTK_PHY_DA_RX_PSBN_LP_MASK,
				      lower_idx << 12 | lower_idx << 8 |
				      lower_idx << 4 | lower_idx);
		ret = lower_ret;
	} else if (upper_idx == TXRESERVE_MAX) {
		upper_ret = cal_cycle(phydev, MDIO_MMD_VEND1,
				      MTK_PHY_RXADC_CTRL_RG9,
				      MTK_PHY_DA_RX_PSBN_TBT_MASK |
				      MTK_PHY_DA_RX_PSBN_HBT_MASK |
				      MTK_PHY_DA_RX_PSBN_GBE_MASK |
				      MTK_PHY_DA_RX_PSBN_LP_MASK,
				      upper_idx << 12 | upper_idx << 8 |
				      upper_idx << 4 | upper_idx);
		ret = upper_ret;
	}
	if (ret < 0)
		goto restore;

	/* We calibrate TX-VCM in different logic. Check upper index and then
	 * lower index. If this calibration is valid, apply lower index's
	 * result.
	 */
	ret = upper_ret - lower_ret;
	if (ret == 1) {
		ret = 0;
		/* Make sure we use upper_idx in our calibration system */
		cal_cycle(phydev, MDIO_MMD_VEND1, MTK_PHY_RXADC_CTRL_RG9,
			  MTK_PHY_DA_RX_PSBN_TBT_MASK |
			  MTK_PHY_DA_RX_PSBN_HBT_MASK |
			  MTK_PHY_DA_RX_PSBN_GBE_MASK |
			  MTK_PHY_DA_RX_PSBN_LP_MASK,
			  upper_idx << 12 | upper_idx << 8 |
			  upper_idx << 4 | upper_idx);
		phydev_dbg(phydev, "TX-VCM SW cal result: 0x%x\n", upper_idx);
	} else if (lower_idx == TXRESERVE_MIN && upper_ret == 1 &&
		   lower_ret == 1) {
		ret = 0;
		cal_cycle(phydev, MDIO_MMD_VEND1, MTK_PHY_RXADC_CTRL_RG9,
			  MTK_PHY_DA_RX_PSBN_TBT_MASK |
			  MTK_PHY_DA_RX_PSBN_HBT_MASK |
			  MTK_PHY_DA_RX_PSBN_GBE_MASK |
			  MTK_PHY_DA_RX_PSBN_LP_MASK,
			  lower_idx << 12 | lower_idx << 8 |
			  lower_idx << 4 | lower_idx);
		phydev_warn(phydev, "TX-VCM SW cal result at low margin 0x%x\n",
			    lower_idx);
	} else if (upper_idx == TXRESERVE_MAX && upper_ret == 0 &&
		   lower_ret == 0) {
		ret = 0;
		phydev_warn(phydev,
			    "TX-VCM SW cal result at high margin 0x%x\n",
			    upper_idx);
	} else {
		ret = -EINVAL;
	}

restore:
	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0,
			   MTK_PHY_RG_ANA_CALEN);
	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG1,
			   MTK_PHY_RG_TXVOS_CALEN);
	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG0,
			   MTK_PHY_RG_ZCALEN_A);
	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_ANA_CAL_RG1,
			   MTK_PHY_RG_ZCALEN_B | MTK_PHY_RG_ZCALEN_C |
			   MTK_PHY_RG_ZCALEN_D);

	return ret;
}

static void mt798x_phy_common_finetune(struct phy_device *phydev)
{
	phy_select_page(phydev, MTK_PHY_PAGE_EXTENDED_52B5);
	__mtk_tr_modify(phydev, 0x1, 0xf, 0x17,
			SLAVE_DSP_READY_TIME_MASK | MASTER_DSP_READY_TIME_MASK,
			FIELD_PREP(SLAVE_DSP_READY_TIME_MASK, 0x18) |
			FIELD_PREP(MASTER_DSP_READY_TIME_MASK, 0x18));

	__mtk_tr_set_bits(phydev, 0x1, 0xf, 0x18,
			  ENABLE_RANDOM_UPDOWN_COUNTER_TRIGGER);

	__mtk_tr_modify(phydev, 0x0, 0x7, 0x15,
			NORMAL_MSE_LO_THRESH_MASK,
			FIELD_PREP(NORMAL_MSE_LO_THRESH_MASK, 0x55));

	__mtk_tr_modify(phydev, 0x2, 0xd, 0x0,
			FFE_UPDATE_GAIN_FORCE_VAL_MASK,
			FIELD_PREP(FFE_UPDATE_GAIN_FORCE_VAL_MASK, 0x4) |
				   FFE_UPDATE_GAIN_FORCE);

	__mtk_tr_clr_bits(phydev, 0x2, 0xd, 0x3, TR_FREEZE_MASK);

	__mtk_tr_modify(phydev, 0x2, 0xd, 0x6,
			SS_TR_KP100_MASK | SS_TR_KF100_MASK |
			SS_TR_KP1000_MASTER_MASK | SS_TR_KF1000_MASTER_MASK |
			SS_TR_KP1000_SLAVE_MASK | SS_TR_KF1000_SLAVE_MASK,
			FIELD_PREP(SS_TR_KP100_MASK, 0x5) |
			FIELD_PREP(SS_TR_KF100_MASK, 0x6) |
			FIELD_PREP(SS_TR_KP1000_MASTER_MASK, 0x5) |
			FIELD_PREP(SS_TR_KF1000_MASTER_MASK, 0x6) |
			FIELD_PREP(SS_TR_KP1000_SLAVE_MASK, 0x5) |
			FIELD_PREP(SS_TR_KF1000_SLAVE_MASK, 0x6));

	phy_restore_page(phydev, MTK_PHY_PAGE_STANDARD, 0);
}

static void mt7981_phy_finetune(struct phy_device *phydev)
{
	u16 val[8] = { 0x01ce, 0x01c1,
		       0x020f, 0x0202,
		       0x03d0, 0x03c0,
		       0x0013, 0x0005 };
	int i, k;

	/* 100M eye finetune:
	 * Keep middle level of TX MLT3 shapper as default.
	 * Only change TX MLT3 overshoot level here.
	 */
	for (k = 0, i = 1; i < 12; i++) {
		if (i % 3 == 0)
			continue;
		phy_write_mmd(phydev, MDIO_MMD_VEND1, i, val[k++]);
	}

	phy_select_page(phydev, MTK_PHY_PAGE_EXTENDED_52B5);
	__mtk_tr_modify(phydev, 0x1, 0xf, 0x20,
			RESET_SYNC_OFFSET_MASK,
			FIELD_PREP(RESET_SYNC_OFFSET_MASK, 0x6));

	__mtk_tr_modify(phydev, 0x1, 0xf, 0x12,
			VGA_DECIMATION_RATE_MASK,
			FIELD_PREP(VGA_DECIMATION_RATE_MASK, 0x1));

	/* MrvlTrFix100Kp = 3, MrvlTrFix100Kf = 2,
	 * MrvlTrFix1000Kp = 3, MrvlTrFix1000Kf = 2
	 */
	__mtk_tr_modify(phydev, 0x1, 0xf, 0x1,
			MRVL_TR_FIX_100KP_MASK | MRVL_TR_FIX_100KF_MASK |
			MRVL_TR_FIX_1000KP_MASK | MRVL_TR_FIX_1000KF_MASK,
			FIELD_PREP(MRVL_TR_FIX_100KP_MASK, 0x3) |
			FIELD_PREP(MRVL_TR_FIX_100KF_MASK, 0x2) |
			FIELD_PREP(MRVL_TR_FIX_1000KP_MASK, 0x3) |
			FIELD_PREP(MRVL_TR_FIX_1000KF_MASK, 0x2));

	/* VcoSlicerThreshBitsHigh */
	__mtk_tr_modify(phydev, 0x1, 0xd, 0x20,
			VCO_SLICER_THRESH_HIGH_MASK,
			FIELD_PREP(VCO_SLICER_THRESH_HIGH_MASK, 0x555555));
	phy_restore_page(phydev, MTK_PHY_PAGE_STANDARD, 0);

	/* TR_OPEN_LOOP_EN = 1, lpf_x_average = 9 */
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DEV1E_REG234,
		       MTK_PHY_TR_OPEN_LOOP_EN_MASK |
		       MTK_PHY_LPF_X_AVERAGE_MASK,
		       BIT(0) | FIELD_PREP(MTK_PHY_LPF_X_AVERAGE_MASK, 0x9));

	/* rg_tr_lpf_cnt_val = 512 */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_LPF_CNT_VAL, 0x200);

	/* IIR2 related */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_LP_IIR2_K1_L, 0x82);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_LP_IIR2_K1_U, 0x0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_LP_IIR2_K2_L, 0x103);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_LP_IIR2_K2_U, 0x0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_LP_IIR2_K3_L, 0x82);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_LP_IIR2_K3_U, 0x0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_LP_IIR2_K4_L, 0xd177);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_LP_IIR2_K4_U, 0x3);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_LP_IIR2_K5_L, 0x2c82);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_LP_IIR2_K5_U, 0xe);

	/* FFE peaking */
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DEV1E_REG27C,
		       MTK_PHY_VGASTATE_FFE_THR_ST1_MASK, 0x1b << 8);
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DEV1E_REG27D,
		       MTK_PHY_VGASTATE_FFE_THR_ST2_MASK, 0x1e);

	/* Disable LDO pump */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_LDO_PUMP_EN_PAIRAB, 0x0);
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_LDO_PUMP_EN_PAIRCD, 0x0);
	/* Adjust LDO output voltage */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_LDO_OUTPUT_V, 0x2222);
}

static void mt7988_phy_finetune(struct phy_device *phydev)
{
	u16 val[12] = { 0x0187, 0x01cd, 0x01c8, 0x0182,
			0x020d, 0x0206, 0x0384, 0x03d0,
			0x03c6, 0x030a, 0x0011, 0x0005 };
	int i;

	/* Set default MLT3 shaper first */
	for (i = 0; i < 12; i++)
		phy_write_mmd(phydev, MDIO_MMD_VEND1, i, val[i]);

	/* TCT finetune */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_TX_FILTER, 0x5);

	phy_select_page(phydev, MTK_PHY_PAGE_EXTENDED_52B5);
	__mtk_tr_modify(phydev, 0x1, 0xf, 0x20,
			RESET_SYNC_OFFSET_MASK,
			FIELD_PREP(RESET_SYNC_OFFSET_MASK, 0x5));

	/* VgaDecRate is 1 at default on mt7988 */

	__mtk_tr_modify(phydev, 0x1, 0xf, 0x1,
			MRVL_TR_FIX_100KP_MASK | MRVL_TR_FIX_100KF_MASK |
			MRVL_TR_FIX_1000KP_MASK | MRVL_TR_FIX_1000KF_MASK,
			FIELD_PREP(MRVL_TR_FIX_100KP_MASK, 0x6) |
			FIELD_PREP(MRVL_TR_FIX_100KF_MASK, 0x7) |
			FIELD_PREP(MRVL_TR_FIX_1000KP_MASK, 0x6) |
			FIELD_PREP(MRVL_TR_FIX_1000KF_MASK, 0x7));

	__mtk_tr_modify(phydev, 0x0, 0xf, 0x3c,
			REMOTE_ACK_COUNT_LIMIT_CTRL_MASK,
			FIELD_PREP(REMOTE_ACK_COUNT_LIMIT_CTRL_MASK, 0x1));
	phy_restore_page(phydev, MTK_PHY_PAGE_STANDARD, 0);

	/* TR_OPEN_LOOP_EN = 1, lpf_x_average = 10 */
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DEV1E_REG234,
		       MTK_PHY_TR_OPEN_LOOP_EN_MASK |
		       MTK_PHY_LPF_X_AVERAGE_MASK,
		       BIT(0) | FIELD_PREP(MTK_PHY_LPF_X_AVERAGE_MASK, 0xa));

	/* rg_tr_lpf_cnt_val = 1023 */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_LPF_CNT_VAL, 0x3ff);
}

static void mt798x_phy_eee(struct phy_device *phydev)
{
	phy_modify_mmd(phydev, MDIO_MMD_VEND1,
		       MTK_PHY_RG_LPI_PCS_DSP_CTRL_REG120,
		       MTK_PHY_LPI_SIG_EN_LO_THRESH1000_MASK |
		       MTK_PHY_LPI_SIG_EN_HI_THRESH1000_MASK,
		       FIELD_PREP(MTK_PHY_LPI_SIG_EN_LO_THRESH1000_MASK, 0x0) |
		       FIELD_PREP(MTK_PHY_LPI_SIG_EN_HI_THRESH1000_MASK, 0x14));

	phy_modify_mmd(phydev, MDIO_MMD_VEND1,
		       MTK_PHY_RG_LPI_PCS_DSP_CTRL_REG122,
		       MTK_PHY_LPI_NORM_MSE_HI_THRESH1000_MASK,
		       FIELD_PREP(MTK_PHY_LPI_NORM_MSE_HI_THRESH1000_MASK,
				  0xff));

	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1,
			   MTK_PHY_RG_TESTMUX_ADC_CTRL,
			   MTK_PHY_RG_TXEN_DIG_MASK);

	phy_set_bits_mmd(phydev, MDIO_MMD_VEND1,
			 MTK_PHY_RG_DEV1E_REG19b, MTK_PHY_BYPASS_DSP_LPI_READY);

	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1,
			   MTK_PHY_RG_DEV1E_REG234, MTK_PHY_TR_LP_IIR_EEE_EN);

	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DEV1E_REG238,
		       MTK_PHY_LPI_SLV_SEND_TX_TIMER_MASK |
		       MTK_PHY_LPI_SLV_SEND_TX_EN,
		       FIELD_PREP(MTK_PHY_LPI_SLV_SEND_TX_TIMER_MASK, 0x120));

	/* Keep MTK_PHY_LPI_SEND_LOC_TIMER as 375 */
	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DEV1E_REG239,
			   MTK_PHY_LPI_TXPCS_LOC_RCV);

	/* This also fixes some IoT issues, such as CH340 */
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DEV1E_REG2C7,
		       MTK_PHY_MAX_GAIN_MASK | MTK_PHY_MIN_GAIN_MASK,
		       FIELD_PREP(MTK_PHY_MAX_GAIN_MASK, 0x8) |
		       FIELD_PREP(MTK_PHY_MIN_GAIN_MASK, 0x13));

	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DEV1E_REG2D1,
		       MTK_PHY_VCO_SLICER_THRESH_BITS_HIGH_EEE_MASK,
		       FIELD_PREP(MTK_PHY_VCO_SLICER_THRESH_BITS_HIGH_EEE_MASK,
				  0x33) |
		       MTK_PHY_LPI_SKIP_SD_SLV_TR | MTK_PHY_LPI_TR_READY |
		       MTK_PHY_LPI_VCO_EEE_STG0_EN);

	phy_set_bits_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DEV1E_REG323,
			 MTK_PHY_EEE_WAKE_MAS_INT_DC |
			 MTK_PHY_EEE_WAKE_SLV_INT_DC);

	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DEV1E_REG324,
		       MTK_PHY_SMI_DETCNT_MAX_MASK,
		       FIELD_PREP(MTK_PHY_SMI_DETCNT_MAX_MASK, 0x3f) |
		       MTK_PHY_SMI_DET_MAX_EN);

	phy_set_bits_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RG_DEV1E_REG326,
			 MTK_PHY_LPI_MODE_SD_ON | MTK_PHY_RESET_RANDUPD_CNT |
			 MTK_PHY_TREC_UPDATE_ENAB_CLR |
			 MTK_PHY_LPI_QUIT_WAIT_DFE_SIG_DET_OFF |
			 MTK_PHY_TR_READY_SKIP_AFE_WAKEUP);

	phy_select_page(phydev, MTK_PHY_PAGE_EXTENDED_52B5);
	__mtk_tr_clr_bits(phydev, 0x2, 0xd, 0x8,
			  EEE1000_SELECT_SIGNAL_DETECTION_FROM_DFE);

	__mtk_tr_modify(phydev, 0x2, 0xd, 0xd,
			EEE1000_STAGE2_TR_KF_MASK,
			FIELD_PREP(EEE1000_STAGE2_TR_KF_MASK, 0x2));

	__mtk_tr_modify(phydev, 0x2, 0xd, 0xf,
			SLAVE_WAKETR_TIMER_MASK | SLAVE_REMTX_TIMER_MASK,
			FIELD_PREP(SLAVE_WAKETR_TIMER_MASK, 0x6) |
			FIELD_PREP(SLAVE_REMTX_TIMER_MASK, 0x14));

	__mtk_tr_modify(phydev, 0x2, 0xd, 0x10,
			SLAVE_WAKEINT_TIMER_MASK,
			FIELD_PREP(SLAVE_WAKEINT_TIMER_MASK, 0x8));

	__mtk_tr_modify(phydev, 0x2, 0xd, 0x14,
			TR_FREEZE_TIMER2_MASK,
			FIELD_PREP(TR_FREEZE_TIMER2_MASK, 0x24a));

	__mtk_tr_modify(phydev, 0x2, 0xd, 0x1c,
			EEE100_LPSYNC_STAGE1_UPDATE_TIMER_MASK,
			FIELD_PREP(EEE100_LPSYNC_STAGE1_UPDATE_TIMER_MASK,
				   0x10));

	__mtk_tr_clr_bits(phydev, 0x2, 0xd, 0x25,
			  WAKE_SLAVE_TR_WAIT_DFE_DETECTION_EN);

	__mtk_tr_modify(phydev, 0x1, 0xf, 0x0,
			DFE_TAIL_EANBLE_VGA_TRHESH_1000,
			FIELD_PREP(DFE_TAIL_EANBLE_VGA_TRHESH_1000, 0x1b));
	phy_restore_page(phydev, MTK_PHY_PAGE_STANDARD, 0);

	phy_select_page(phydev, MTK_PHY_PAGE_EXTENDED_3);
	__phy_modify(phydev, MTK_PHY_LPI_REG_14,
		     MTK_PHY_LPI_WAKE_TIMER_1000_MASK,
		     FIELD_PREP(MTK_PHY_LPI_WAKE_TIMER_1000_MASK, 0x19c));

	__phy_modify(phydev, MTK_PHY_LPI_REG_1c, MTK_PHY_SMI_DET_ON_THRESH_MASK,
		     FIELD_PREP(MTK_PHY_SMI_DET_ON_THRESH_MASK, 0xc));
	phy_restore_page(phydev, MTK_PHY_PAGE_STANDARD, 0);

	phy_modify_mmd(phydev, MDIO_MMD_VEND1,
		       MTK_PHY_RG_LPI_PCS_DSP_CTRL_REG122,
		       MTK_PHY_LPI_NORM_MSE_HI_THRESH1000_MASK,
		       FIELD_PREP(MTK_PHY_LPI_NORM_MSE_HI_THRESH1000_MASK,
				  0xff));
}

static int cal_sw(struct phy_device *phydev, enum CAL_ITEM cal_item,
		  u8 start_pair, u8 end_pair)
{
	u8 pair_n;
	int ret;

	for (pair_n = start_pair; pair_n <= end_pair; pair_n++) {
		/* TX_OFFSET & TX_AMP have no SW calibration. */
		switch (cal_item) {
		case TX_VCM:
			ret = tx_vcm_cal_sw(phydev, pair_n);
			break;
		default:
			return -EINVAL;
		}
		if (ret)
			return ret;
	}
	return 0;
}

static int cal_efuse(struct phy_device *phydev, enum CAL_ITEM cal_item,
		     u8 start_pair, u8 end_pair, u32 *buf)
{
	u8 pair_n;
	int ret;

	for (pair_n = start_pair; pair_n <= end_pair; pair_n++) {
		/* TX_VCM has no efuse calibration. */
		switch (cal_item) {
		case REXT:
			ret = rext_cal_efuse(phydev, buf);
			break;
		case TX_OFFSET:
			ret = tx_offset_cal_efuse(phydev, buf);
			break;
		case TX_AMP:
			ret = tx_amp_cal_efuse(phydev, buf);
			break;
		case TX_R50:
			ret = tx_r50_cal_efuse(phydev, buf, pair_n);
			break;
		default:
			return -EINVAL;
		}
		if (ret)
			return ret;
	}

	return 0;
}

static int start_cal(struct phy_device *phydev, enum CAL_ITEM cal_item,
		     enum CAL_MODE cal_mode, u8 start_pair,
		     u8 end_pair, u32 *buf)
{
	int ret;

	switch (cal_mode) {
	case EFUSE_M:
		ret = cal_efuse(phydev, cal_item, start_pair,
				end_pair, buf);
		break;
	case SW_M:
		ret = cal_sw(phydev, cal_item, start_pair, end_pair);
		break;
	default:
		return -EINVAL;
	}

	if (ret) {
		phydev_err(phydev, "cal %d failed\n", cal_item);
		return -EIO;
	}

	return 0;
}

static int mt798x_phy_calibration(struct phy_device *phydev)
{
	struct nvmem_cell *cell;
	int ret = 0;
	size_t len;
	u32 *buf;

	cell = nvmem_cell_get(&phydev->mdio.dev, "phy-cal-data");
	if (IS_ERR(cell)) {
		if (PTR_ERR(cell) == -EPROBE_DEFER)
			return PTR_ERR(cell);
		return 0;
	}

	buf = (u32 *)nvmem_cell_read(cell, &len);
	if (IS_ERR(buf))
		return PTR_ERR(buf);
	nvmem_cell_put(cell);

	if (!buf[0] || !buf[1] || !buf[2] || !buf[3] || len < 4 * sizeof(u32)) {
		phydev_err(phydev, "invalid efuse data\n");
		ret = -EINVAL;
		goto out;
	}

	ret = start_cal(phydev, REXT, EFUSE_M, NO_PAIR, NO_PAIR, buf);
	if (ret)
		goto out;
	ret = start_cal(phydev, TX_OFFSET, EFUSE_M, NO_PAIR, NO_PAIR, buf);
	if (ret)
		goto out;
	ret = start_cal(phydev, TX_AMP, EFUSE_M, NO_PAIR, NO_PAIR, buf);
	if (ret)
		goto out;
	ret = start_cal(phydev, TX_R50, EFUSE_M, PAIR_A, PAIR_D, buf);
	if (ret)
		goto out;
	ret = start_cal(phydev, TX_VCM, SW_M, PAIR_A, PAIR_A, buf);
	if (ret)
		goto out;

out:
	kfree(buf);
	return ret;
}

static int mt798x_phy_config_init(struct phy_device *phydev)
{
	switch (phydev->drv->phy_id) {
	case MTK_GPHY_ID_MT7981:
		mt7981_phy_finetune(phydev);
		break;
	case MTK_GPHY_ID_MT7988:
		mt7988_phy_finetune(phydev);
		break;
	}

	mt798x_phy_common_finetune(phydev);
	mt798x_phy_eee(phydev);

	return mt798x_phy_calibration(phydev);
}

static int mt798x_phy_led_blink_set(struct phy_device *phydev, u8 index,
				    unsigned long *delay_on,
				    unsigned long *delay_off)
{
	bool blinking = false;
	int err;

	err = mtk_phy_led_num_dly_cfg(index, delay_on, delay_off, &blinking);
	if (err < 0)
		return err;

	err = mtk_phy_hw_led_blink_set(phydev, index, blinking);
	if (err)
		return err;

	return mtk_phy_hw_led_on_set(phydev, index, MTK_GPHY_LED_ON_MASK,
				     false);
}

static int mt798x_phy_led_brightness_set(struct phy_device *phydev,
					 u8 index, enum led_brightness value)
{
	int err;

	err = mtk_phy_hw_led_blink_set(phydev, index, false);
	if (err)
		return err;

	return mtk_phy_hw_led_on_set(phydev, index, MTK_GPHY_LED_ON_MASK,
				     (value != LED_OFF));
}

static const unsigned long supported_triggers =
	BIT(TRIGGER_NETDEV_FULL_DUPLEX) |
	BIT(TRIGGER_NETDEV_HALF_DUPLEX) |
	BIT(TRIGGER_NETDEV_LINK)        |
	BIT(TRIGGER_NETDEV_LINK_10)     |
	BIT(TRIGGER_NETDEV_LINK_100)    |
	BIT(TRIGGER_NETDEV_LINK_1000)   |
	BIT(TRIGGER_NETDEV_RX)          |
	BIT(TRIGGER_NETDEV_TX);

static int mt798x_phy_led_hw_is_supported(struct phy_device *phydev, u8 index,
					  unsigned long rules)
{
	return mtk_phy_led_hw_is_supported(phydev, index, rules,
					   supported_triggers);
}

static int mt798x_phy_led_hw_control_get(struct phy_device *phydev, u8 index,
					 unsigned long *rules)
{
	return mtk_phy_led_hw_ctrl_get(phydev, index, rules,
				       MTK_GPHY_LED_ON_SET,
				       MTK_GPHY_LED_RX_BLINK_SET,
				       MTK_GPHY_LED_TX_BLINK_SET);
};

static int mt798x_phy_led_hw_control_set(struct phy_device *phydev, u8 index,
					 unsigned long rules)
{
	return mtk_phy_led_hw_ctrl_set(phydev, index, rules,
				       MTK_GPHY_LED_ON_SET,
				       MTK_GPHY_LED_RX_BLINK_SET,
				       MTK_GPHY_LED_TX_BLINK_SET);
};

static bool mt7988_phy_led_get_polarity(struct phy_device *phydev, int led_num)
{
	struct mtk_socphy_shared *priv = phy_package_get_priv(phydev);
	u32 polarities;

	if (led_num == 0)
		polarities = ~(priv->boottrap);
	else
		polarities = MTK_PHY_LED1_DEFAULT_POLARITIES;

	if (polarities & BIT(phydev->mdio.addr))
		return true;

	return false;
}

static int mt7988_phy_fix_leds_polarities(struct phy_device *phydev)
{
	struct pinctrl *pinctrl;
	int index;

	/* Setup LED polarity according to bootstrap use of LED pins */
	for (index = 0; index < 2; ++index)
		phy_modify_mmd(phydev, MDIO_MMD_VEND2, index ?
				MTK_PHY_LED1_ON_CTRL : MTK_PHY_LED0_ON_CTRL,
			       MTK_PHY_LED_ON_POLARITY,
			       mt7988_phy_led_get_polarity(phydev, index) ?
				MTK_PHY_LED_ON_POLARITY : 0);

	/* Only now setup pinctrl to avoid bogus blinking */
	pinctrl = devm_pinctrl_get_select(&phydev->mdio.dev, "gbe-led");
	if (IS_ERR(pinctrl))
		dev_err(&phydev->mdio.bus->dev,
			"Failed to setup PHY LED pinctrl\n");

	return 0;
}

static int mt7988_phy_probe_shared(struct phy_device *phydev)
{
	struct device_node *np = dev_of_node(&phydev->mdio.bus->dev);
	struct mtk_socphy_shared *shared = phy_package_get_priv(phydev);
	struct device_node *pio_np;
	struct regmap *regmap;
	u32 reg;
	int ret;

	/* The LED0 of the 4 PHYs in MT7988 are wired to SoC pins LED_A, LED_B,
	 * LED_C and LED_D respectively. At the same time those pins are used to
	 * bootstrap configuration of the reference clock source (LED_A),
	 * DRAM DDRx16b x2/x1 (LED_B) and boot device (LED_C, LED_D).
	 * In practice this is done using a LED and a resistor pulling the pin
	 * either to GND or to VIO.
	 * The detected value at boot time is accessible at run-time using the
	 * TPBANK0 register located in the gpio base of the pinctrl, in order
	 * to read it here it needs to be referenced by a phandle called
	 * 'mediatek,pio' in the MDIO bus hosting the PHY.
	 * The 4 bits in TPBANK0 are kept as package shared data and are used to
	 * set LED polarity for each of the LED0.
	 */
	pio_np = of_parse_phandle(np, "mediatek,pio", 0);
	if (!pio_np)
		return -ENODEV;

	regmap = device_node_to_regmap(pio_np);
	of_node_put(pio_np);

	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	ret = regmap_read(regmap, RG_GPIO_MISC_TPBANK0, &reg);
	if (ret)
		return ret;

	shared->boottrap = FIELD_GET(RG_GPIO_MISC_TPBANK0_BOOTMODE, reg);

	return 0;
}

static int mt7988_phy_probe(struct phy_device *phydev)
{
	struct mtk_socphy_shared *shared;
	struct mtk_socphy_priv *priv;
	int err;

	if (phydev->mdio.addr > 3)
		return -EINVAL;

	err = devm_phy_package_join(&phydev->mdio.dev, phydev, 0,
				    sizeof(struct mtk_socphy_shared));
	if (err)
		return err;

	if (phy_package_probe_once(phydev)) {
		err = mt7988_phy_probe_shared(phydev);
		if (err)
			return err;
	}

	shared = phy_package_get_priv(phydev);
	priv = &shared->priv[phydev->mdio.addr];

	phydev->priv = priv;

	mtk_phy_leds_state_init(phydev);

	err = mt7988_phy_fix_leds_polarities(phydev);
	if (err)
		return err;

	/* Disable TX power saving at probing to:
	 * 1. Meet common mode compliance test criteria
	 * 2. Make sure that TX-VCM calibration works fine
	 */
	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_RXADC_CTRL_RG7,
		       MTK_PHY_DA_AD_BUF_BIAS_LP_MASK, 0x3 << 8);

	return mt798x_phy_calibration(phydev);
}

static int mt7981_phy_probe(struct phy_device *phydev)
{
	struct mtk_socphy_priv *priv;

	priv = devm_kzalloc(&phydev->mdio.dev, sizeof(struct mtk_socphy_priv),
			    GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	phydev->priv = priv;

	mtk_phy_leds_state_init(phydev);

	return mt798x_phy_calibration(phydev);
}

static struct phy_driver mtk_socphy_driver[] = {
	{
		PHY_ID_MATCH_EXACT(MTK_GPHY_ID_MT7981),
		.name		= "MediaTek MT7981 PHY",
		.config_init	= mt798x_phy_config_init,
		.config_intr	= genphy_no_config_intr,
		.handle_interrupt = genphy_handle_interrupt_no_ack,
		.probe		= mt7981_phy_probe,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.read_page	= mtk_phy_read_page,
		.write_page	= mtk_phy_write_page,
		.led_blink_set	= mt798x_phy_led_blink_set,
		.led_brightness_set = mt798x_phy_led_brightness_set,
		.led_hw_is_supported = mt798x_phy_led_hw_is_supported,
		.led_hw_control_set = mt798x_phy_led_hw_control_set,
		.led_hw_control_get = mt798x_phy_led_hw_control_get,
	},
	{
		PHY_ID_MATCH_EXACT(MTK_GPHY_ID_MT7988),
		.name		= "MediaTek MT7988 PHY",
		.config_init	= mt798x_phy_config_init,
		.config_intr	= genphy_no_config_intr,
		.handle_interrupt = genphy_handle_interrupt_no_ack,
		.probe		= mt7988_phy_probe,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.read_page	= mtk_phy_read_page,
		.write_page	= mtk_phy_write_page,
		.led_blink_set	= mt798x_phy_led_blink_set,
		.led_brightness_set = mt798x_phy_led_brightness_set,
		.led_hw_is_supported = mt798x_phy_led_hw_is_supported,
		.led_hw_control_set = mt798x_phy_led_hw_control_set,
		.led_hw_control_get = mt798x_phy_led_hw_control_get,
	},
};

module_phy_driver(mtk_socphy_driver);

static const struct mdio_device_id __maybe_unused mtk_socphy_tbl[] = {
	{ PHY_ID_MATCH_EXACT(MTK_GPHY_ID_MT7981) },
	{ PHY_ID_MATCH_EXACT(MTK_GPHY_ID_MT7988) },
	{ }
};

MODULE_DESCRIPTION("MediaTek SoC Gigabit Ethernet PHY driver");
MODULE_AUTHOR("Daniel Golle <daniel@makrotopia.org>");
MODULE_AUTHOR("SkyLake Huang <SkyLake.Huang@mediatek.com>");
MODULE_LICENSE("GPL");

MODULE_DEVICE_TABLE(mdio, mtk_socphy_tbl);
