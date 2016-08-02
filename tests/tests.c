#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "n_rf24l01.h"


//
//======================================================================================================
void read_all_registers( void )
{
	int ret;
	__u64 reg = 0;
	__u8 status_reg;

	//--------------- read 5-bytes registers
	ret = send_command( R_REGISTER | RX_ADDR_P0_RG, NULL, &reg, 5, 0 );
	if( ret < 0 ) return;
	printf( "RX_ADDR_P0_RG: 0x%llx.\n", reg );

	ret = send_command( R_REGISTER | RX_ADDR_P1_RG, NULL, &reg, 5, 0 );
	if( ret < 0 ) return;
	printf( "RX_ADDR_P1_RG: 0x%llx.\n", reg );

	ret = send_command( R_REGISTER | TX_ADDR_RG, NULL, &reg, 5, 0 );
	if( ret < 0 ) return;
	printf( "TX_ADDR_RG: 0x%llx.\n", reg );

	//--------------- read 1-byte registers
	reg = 0;

	ret = send_command( R_REGISTER | CONFIG_RG, NULL, &reg, 1, 0 );
	if( ret < 0 ) return;
	status_reg = (__u8)reg ;
	printf( "CONFIG_RG: 0x%02hhx.\n", (__u8)reg );

	ret = send_command( R_REGISTER | EN_AA_RG, NULL, &reg, 1, 0 );
	if( ret < 0 ) return;
	printf( "EN_AA_RG: 0x%02hhx.\n",  (__u8)reg );

	ret = send_command( R_REGISTER | STATUS_RG, &status_reg, &reg, 1, 0 );
	if( ret < 0 ) return;
	printf( "STATUS_RG: 0x%02hhx.\n",  (__u8)reg );
	printf( "STATUS_RG: 0x%02hhx.\n\n", status_reg );
}

//
//======================================================================================================
void test_set_clear_read_write( void )
{
	__u8 reg;

	read_register( CONFIG_RG, &reg );
	printf( "CONFIG_RG: 0x%02hhx.\n", reg );

	set_bits( CONFIG_RG, PWR_UP );

	read_register( CONFIG_RG, &reg );
	printf( "CONFIG_RG: 0x%02hhx.\n", reg );

	clear_bits( CONFIG_RG, PWR_UP );

	read_register( CONFIG_RG, &reg );
	printf( "CONFIG_RG: 0x%02hhx.\n\n", reg );

	write_register( CONFIG_RG, 0x0f );

	read_register( CONFIG_RG, &reg );
	printf( "CONFIG_RG: 0x%02hhx.\n\n", reg );
}

//
//======================================================================================================
int main( void )
{
	int ret;

	ret = init_n_rf24l01();
	if( ret < 0 ) return 1;

	test_set_clear_read_write();
	read_all_registers();

	return 0;
}
