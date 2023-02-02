#include"hardware/ata.h"
#include"io.h"
#include"mem/malloc.h"
#include"gates/irq.h"
#include"hardware/timer.h"
#include"console.h"
#include"string.h"
#include"hardware/devices.h"
#include"process/sync.h"
volatile unsigned static char ide_irq_invoked = 0;
unsigned static char atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static lock_t ata_lock;
struct IdeIdentifyInfo {
	//	0	General configuration bit-significant information
	unsigned short General_Config;

	//	1	Obsolete
	unsigned short Obsolete0;

	//	2	Specific configuration
	unsigned short Specific_Coinfig;

	//	3	Obsolete
	unsigned short Obsolete1;

	//	4-5	Retired
	unsigned short Retired0[2];

	//	6	Obsolete
	unsigned short Obsolete2;

	//	7-8	Reserved for the CompactFlash Association
	unsigned short CompactFlash[2];

	//	9	Retired
	unsigned short Retired1;

	//	10-19	Serial number (20 ASCII characters)
	unsigned short Serial_Number[10];

	//	20-21	Retired
	unsigned short Retired2[2];

	//	22	Obsolete
	unsigned short Obsolete3;

	//	23-26	Firmware revision(8 ASCII characters)
	unsigned short Firmware_Version[4];

	//	27-46	Model number (40 ASCII characters)
	unsigned short Model_Number[20];

	//	47	15:8 	80h 
	//		7:0  	00h=Reserved 
	//			01h-FFh = Maximumnumber of logical sectors that shall be transferred per DRQ data block on READ/WRITE MULTIPLE commands
	unsigned short Max_logical_transferred_per_DRQ;

	//	48	Trusted Computing feature set options
	unsigned short Trusted_Computing_feature_set_options;

	//	49	Capabilities
	unsigned short Capabilities0;

	//	50	Capabilities
	unsigned short Capabilities1;

	//	51-52	Obsolete
	unsigned short Obsolete4[2];

	//	53	15:8	Free-fall Control Sensitivity
	//		7:3 	Reserved
	//		2 	the fields reported in word 88 are valid
	//		1 	the fields reported in words (70:64) are valid
	unsigned short Report_88_70to64_valid;

	//	54-58	Obsolete
	unsigned short Obsolete5[5];

	//	59	15:9	Reserved
	//		8	Multiple sector setting is valid	
	//		7:0	xxh current setting for number of logical sectors that shall be transferred per DRQ data block on READ/WRITE Multiple commands
	unsigned short Mul_Sec_Setting_Valid;

	//	60-61	Total number of user addresssable logical sectors for 28bit CMD
	unsigned short lba28Sectors[2];

	//	62	Obsolete
	unsigned short Obsolete6;

	//	63	15:11	Reserved
	//		10:8=1 	Multiword DMA mode 210 is selected
	//		7:3 	Reserved
	//		2:0=1 	Multiword DMA mode 210 and below are supported
	unsigned short MultWord_DMA_Select;

	//	64	15:8	Reserved
	//		7:0	PIO mdoes supported
	unsigned short PIO_mode_supported;

	//	65	Minimum Multiword DMA transfer cycle time per word
	unsigned short Min_MulWord_DMA_cycle_time_per_word;

	//	66	Manufacturer`s recommended Multiword DMA transfer cycle time
	unsigned short Manufacture_Recommend_MulWord_DMA_cycle_time;

	//	67	Minimum PIO transfer cycle time without flow control
	unsigned short Min_PIO_cycle_time_Flow_Control;

	//	68	Minimum PIO transfer cycle time with IORDY flow control
	unsigned short Min_PIO_cycle_time_IOREDY_Flow_Control;

	//	69-70	Reserved
	unsigned short Reserved1[2];

	//	71-74	Reserved for the IDENTIFY PACKET DEVICE command
	unsigned short Reserved2[4];

	//	75	Queue depth
	unsigned short Queue_depth;

	//	76	Serial ATA Capabilities 
	unsigned short SATA_Capabilities;

	//	77	Reserved for Serial ATA 
	unsigned short Reserved3;

	//	78	Serial ATA features Supported 
	unsigned short SATA_features_Supported;

	//	79	Serial ATA features enabled
	unsigned short SATA_features_enabled;

	//	80	Major Version number
	unsigned short Major_Version;

	//	81	Minor version number
	unsigned short Minor_Version;

	//	82	Commands and feature sets supported
	unsigned short cmdSet0;

	//	83	Commands and feature sets supported	
	unsigned short cmdSet1;

	//	84	Commands and feature sets supported
	unsigned short Cmd_feature_sets_supported2;

	//	85	Commands and feature sets supported or enabled
	unsigned short Cmd_feature_sets_supported3;

	//	86	Commands and feature sets supported or enabled
	unsigned short Cmd_feature_sets_supported4;

	//	87	Commands and feature sets supported or enabled
	unsigned short Cmd_feature_sets_supported5;

	//	88	15 	Reserved 
	//		14:8=1 	Ultra DMA mode 6543210 is selected 
	//		7 	Reserved 
	//		6:0=1 	Ultra DMA mode 6543210 and below are suported
	unsigned short Ultra_DMA_modes;

	//	89	Time required for Normal Erase mode SECURITY ERASE UNIT command
	unsigned short Time_required_Erase_CMD;

	//	90	Time required for an Enhanced Erase mode SECURITY ERASE UNIT command
	unsigned short Time_required_Enhanced_CMD;

	//	91	Current APM level value
	unsigned short Current_APM_level_Value;

	//	92	Master Password Identifier
	unsigned short Master_Password_Identifier;

	//	93	Hardware resset result.The contents of bits (12:0) of this word shall change only during the execution of a hardware reset.
	unsigned short HardWare_Reset_Result;

	//	94	Current AAM value 
	//		15:8 	Vendor’s recommended AAM value 
	//		7:0 	Current AAM value
	unsigned short Current_AAM_value;

	//	95	Stream Minimum Request Size
	unsigned short Stream_Min_Request_Size;

	//	96	Streaming Transger Time-DMA 
	unsigned short Streaming_Transger_time_DMA;

	//	97	Streaming Access Latency-DMA and PIO
	unsigned short Streaming_Access_Latency_DMA_PIO;

	//	98-99	Streaming Performance Granularity (DWord)
	unsigned short Streaming_Performance_Granularity[2];

	//	100-103	Total Number of User Addressable Logical Sectors for 48-bit commands (QWord)
	unsigned short lba48Sectors[4];

	//	104	Streaming Transger Time-PIO
	unsigned short Streaming_Transfer_Time_PIO;

	//	105	Reserved
	unsigned short Reserved4;

	//	106	Physical Sector size/Logical Sector Size
	unsigned short Physical_Logical_Sector_Size;

	//	107	Inter-seek delay for ISO-7779 acoustic testing in microseconds
	unsigned short Inter_seek_delay;

	//	108-111	World wide name	
	unsigned short World_wide_name[4];

	//	112-115	Reserved
	unsigned short Reserved5[4];

	//	116	Reserved for TLC
	unsigned short Reserved6;

	//	117-118	Logical sector size (DWord)
	unsigned short Words_per_Logical_Sector[2];

	//	119	Commands and feature sets supported (Continued from words 84:82)
	unsigned short CMD_feature_Supported;

	//	120	Commands and feature sets supported or enabled (Continued from words 87:85)
	unsigned short CMD_feature_Supported_enabled;

	//	121-126	Reserved for expanded supported and enabled settings
	unsigned short Reserved7[6];

	//	127	Obsolete
	unsigned short Obsolete7;

	//	128	Security status
	unsigned short Security_Status;

	//	129-159	Vendor specific
	unsigned short Vendor_Specific[31];

	//	160	CFA power mode
	unsigned short CFA_Power_mode;

	//	161-167	Reserved for the CompactFlash Association
	unsigned short Reserved8[7];

	//	168	Device Nominal Form Factor
	unsigned short Dev_from_Factor;

	//	169-175	Reserved
	unsigned short Reserved9[7];

	//	176-205	Current media serial number (ATA string)
	unsigned short Current_Media_Serial_Number[30];

	//	206	SCT Command Transport
	unsigned short SCT_Cmd_Transport;

	//	207-208	Reserved for CE-ATA
	unsigned short Reserved10[2];

	//	209	Alignment of logical blocks within a physical block
	unsigned short Alignment_Logical_blocks_within_a_physical_block;

	//	210-211	Write-Read-Verify Sector Count Mode 3 (DWord)
	unsigned short Write_Read_Verify_Sector_Count_Mode_3[2];

	//	212-213	Write-Read-Verify Sector Count Mode 2 (DWord)
	unsigned short Write_Read_Verify_Sector_Count_Mode_2[2];

	//	214	NV Cache Capabilities
	unsigned short NV_Cache_Capabilities;

	//	215-216	NV Cache Size in Logical Blocks (DWord)
	unsigned short NV_Cache_Size[2];

	//	217	Nominal media rotation rate
	unsigned short Nominal_media_rotation_rate;

	//	218	Reserved
	unsigned short Reserved11;

	//	219	NV Cache Options
	unsigned short NV_Cache_Options;

	//	220	Write-Read-Verify feature set current mode
	unsigned short Write_Read_Verify_feature_set_current_mode;

	//	221	Reserved
	unsigned short Reserved12;

	//	222	Transport major version number. 
	//		0000h or ffffh = device does not report version
	unsigned short Transport_Major_Version_Number;

	//	223	Transport Minor version number
	unsigned short Transport_Minor_Version_Number;

	//	224-233	Reserved for CE-ATA
	unsigned short Reserved13[10];

	//	234	Minimum number of 512-byte data blocks per DOWNLOAD MICROCODE command for mode 03h
	unsigned short Mini_blocks_per_CMD;

	//	235	Maximum number of 512-byte data blocks per DOWNLOAD MICROCODE command for mode 03h
	unsigned short Max_blocks_per_CMD;

	//	236-254	Reserved
	unsigned short Reserved14[19];

	//	255	Integrity word
	//		15:8	Checksum
	//		7:0	Checksum Validity Indicator
	unsigned short Integrity_word;
}__attribute__((packed));
unsigned char ide_buf[2048] = {0};
typedef struct 
{
   unsigned short base;  // I/O Base.
   unsigned short ctrl;  // Control Base
   unsigned short bmide; // Bus Master IDE
   unsigned char  nIEN;  // nIEN (No Interrupt);
}ata_channel_t;
ata_channel_t channels[2];
typedef struct {
  uint16_t    config;                                ///< General Configuration.
  uint16_t    reserved_1;
  uint16_t    specific_config;                       ///< Specific Configuration.
  uint16_t    reserved_3_9[7];
  uint8_t     SerialNo[20];                          ///< word 10~19
  uint16_t    reserved_20_22[3];
  uint8_t     FirmwareVer[8];                        ///< word 23~26
  uint8_t     ModelName[40];                         ///< word 27~46
  uint16_t    reserved_47_48[2];
  uint16_t    capabilities_49;
  uint16_t    capabilities_50;
  uint16_t    obsolete_51;
  uint16_t    reserved_52;
  uint16_t    field_validity;                        ///< word 53
  uint16_t    reserved_54_61[8];
  uint16_t    dma_dir;
  uint16_t    multi_word_dma_mode;                   ///< word 63
  uint16_t    advanced_pio_modes;                    ///< word 64
  uint16_t    min_multi_word_dma_cycle_time;
  uint16_t    rec_multi_word_dma_cycle_time;
  uint16_t    min_pio_cycle_time_without_flow_control;
  uint16_t    min_pio_cycle_time_with_flow_control;
  uint16_t    reserved_69_70[2];
  uint16_t    obsolete_71_72[2];
  uint16_t    reserved_73_74[2];
  uint16_t    obsolete_75;
  uint16_t    serial_ata_capabilities;
  uint16_t    reserved_77;                           ///< Reserved for Serial ATA
  uint16_t    serial_ata_features_supported;
  uint16_t    serial_ata_features_enabled;
  uint16_t    major_version_no;                      ///< word 80
  uint16_t    minor_version_no;                      ///< word 81
  uint16_t    cmd_set_support_82;
  uint16_t    cmd_set_support_83;
  uint16_t    cmd_feature_support;
  uint16_t    cmd_feature_enable_85;
  uint16_t    cmd_feature_enable_86;
  uint16_t    cmd_feature_default;
  uint16_t    ultra_dma_select;
  uint16_t    time_required_for_sec_erase;           ///< word 89
  uint16_t    time_required_for_enhanced_sec_erase;  ///< word 90
  uint16_t    advanced_power_management_level;
  uint16_t    master_pwd_revison_code;
  uint16_t    hardware_reset_result;                 ///< word 93
  uint16_t    obsolete_94;
  uint16_t    reserved_95_107[13];
  uint16_t    world_wide_name[4];                    ///< word 108~111
  uint16_t    reserved_for_128bit_wwn_112_115[4];
  uint16_t    reserved_116_118[3];
  uint16_t    command_and_feature_sets_supported;    ///< word 119
  uint16_t    command_and_feature_sets_supported_enabled;
  uint16_t    reserved_121_124[4];
  uint16_t    atapi_byte_count_0_behavior;           ///< word 125
  uint16_t    obsolete_126_127[2];
  uint16_t    security_status;
  uint16_t    reserved_129_159[31];
  uint16_t    cfa_reserved_160_175[16];
  uint16_t    reserved_176_221[46];
  uint16_t    transport_major_version;
  uint16_t    transport_minor_version;
  uint16_t    reserved_224_254[31];
  uint16_t    integrity_word;
} ATAPI_IDENTIFY_DATA;
typedef struct  {
   unsigned char  Reserved;    // 0 (Empty) or 1 (This Drive really exists).
   unsigned char  Channel;     // 0 (Primary Channel) or 1 (Secondary Channel).
   unsigned char  Drive;       // 0 (Master Drive) or 1 (Slave Drive).
   unsigned short Type;        // 0: ATA, 1:ATAPI.
   unsigned short Signature;   // Drive Signature
   unsigned short Capabilities;// Features.
   unsigned int   CommandSets; // Command Sets Supported.
   unsigned int   MaxLBA;        // Size in Sectors.
   uint32_t UnitSize;
   unsigned char  Model[41];   // Model in string.
}ide_device;
ide_device ide_devices[4];
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

#define ATA_ER_BBK      0x80    // Bad block
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // Media changed
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // Media change request
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define      ATAPI_CMD_READ       0xA8
#define      ATAPI_CMD_EJECT      0x1B

#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01
 
#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

#define      ATA_PRIMARY      0x00
#define      ATA_SECONDARY    0x01
 
// Directions:
#define      ATA_READ      0x00
#define      ATA_WRITE     0x01
bool irq_called=0;
void ide_irq(registers_t reg)
{
   irq_called=1;
}
void wait_for_irq()
{
   while (!irq_called)
   {
      /* code */
   }
   irq_called=0;
   
}
void ide_write(unsigned char channel, unsigned char reg, unsigned char data) {
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
   if (reg < 0x08)
      outb(channels[channel].base  + reg - 0x00, data);
   else if (reg < 0x0C)
      outb(channels[channel].base  + reg - 0x06, data);
   else if (reg < 0x0E)
      outb(channels[channel].ctrl  + reg - 0x0C, data);
   else if (reg < 0x16)
      outb(channels[channel].bmide + reg - 0x0E, data);
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}
unsigned char ide_read(unsigned char channel, unsigned char reg) {
   unsigned char result;
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
   if (reg < 0x08)
      result = inb(channels[channel].base + reg - 0x00);
   else if (reg < 0x0C)
      result = inb(channels[channel].base  + reg - 0x06);
   else if (reg < 0x0E)
      result = inb(channels[channel].ctrl  + reg - 0x0C);
   else if (reg < 0x16)
      result = inb(channels[channel].bmide + reg - 0x0E);
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
      //sizeof(IDENTIFY_DEVICE_DATA);
   return result;
}

void ide_read_buffer(unsigned char channel, unsigned char reg, unsigned int buffer,
                     unsigned int quads) {
   /* WARNING: This code contains a serious bug. The inline assembly trashes ES and
    *           ESP for all of the code the compiler generates between the inline
    *           assembly blocks.
    */
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
   asm("pushw %es \n\t"
   "pushw %ax \n\t"
   "movw %ds, %ax \n\t"
   "movw %ax, %es \n\t"
   "popw %ax \n\t"); 
   if (reg < 0x08)
      insl(channels[channel].base  + reg - 0x00, buffer, quads);
   else if (reg < 0x0C)
      insl(channels[channel].base  + reg - 0x06, buffer, quads);
   else if (reg < 0x0E)
      insl(channels[channel].ctrl  + reg - 0x0C, buffer, quads);
   else if (reg < 0x16)
      insl(channels[channel].bmide + reg - 0x0E, buffer, quads);
   asm("popw %es");
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}
unsigned char ide_polling(unsigned char channel, unsigned int advanced_check) {
 
   // (I) Delay 400 nanosecond for BSY to be set:
   // -------------------------------------------------
   for(int i = 0; i < 4; i++)
      ide_read(channel, ATA_REG_ALTSTATUS); // Reading the Alternate Status port wastes 100ns; loop four times.
 
   // (II) Wait for BSY to be cleared:
   // -------------------------------------------------
   while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY)
      ; // Wait for BSY to be zero.
 
   if (advanced_check) {
      unsigned char state = ide_read(channel, ATA_REG_STATUS); // Read Status Register.
 
      // (III) Check For Errors:
      // -------------------------------------------------
      if (state & ATA_SR_ERR)
         return 2; // Error.
 
      // (IV) Check If Device fault:
      // -------------------------------------------------
      if (state & ATA_SR_DF)
         return 1; // Device Fault.
 
      // (V) Check DRQ:
      // -------------------------------------------------
      // BSY = 0; DF = 0; ERR = 0 so we should check for DRQ now.
      if ((state & ATA_SR_DRQ) == 0)
         return 3; // DRQ should be set
 
   }
 
   return 0; // No Error.
 
}
typedef struct 
{
   uint32_t max_lba;
   uint32_t unit_size;
}atapi_capacity_t;

unsigned char ide_print_error(unsigned int drive, unsigned char err) {
   if (err == 0)
      return err;
   if(err==114)
   {
      printf("- No Support For CHS Mode!\n   " );
      return err;
   }else if(err==113)
   {
      printf("- Bad Unit Size!\n ");
      return err;
   }
   printf("IDE:");
   if (err == 1) {printf("- Device Fault\n     "); err = 19;}
   else if (err == 2) {
      unsigned char st = ide_read(ide_devices[drive].Channel, ATA_REG_ERROR);
      if (st & ATA_ER_AMNF)   {printf("- No Address Mark Found\n     ");   err = 7;}
      if (st & ATA_ER_TK0NF)   {printf("- No Media or Media Error\n     ");   err = 3;}
      if (st & ATA_ER_ABRT)   {printf("- Command Aborted\n     ");      err = 20;}
      if (st & ATA_ER_MCR)   {printf("- No Media or Media Error\n     ");   err = 3;}
      if (st & ATA_ER_IDNF)   {printf("- ID mark not Found\n     ");      err = 21;}
      if (st & ATA_ER_MC)   {printf("- No Media or Media Error\n     ");   err = 3;}
      if (st & ATA_ER_UNC)   {printf("- Uncorrectable Data Error\n     ");   err = 22;}
      if (st & ATA_ER_BBK)   {printf("- Bad Sectors\n     ");       err = 13;}
   } else  if (err == 3)           {printf("- Reads Nothing\n     "); err = 23;}
     else  if (err == 4)  {printf("- Write Protected\n     "); err = 8;}
   printf("- [%s %s] %s\n",
      (const char *[]){"Primary", "Secondary"}[ide_devices[drive].Channel], // Use the channel as an index into the array
      (const char *[]){"Master", "Slave"}[ide_devices[drive].Drive], // Same as above, using the drive
      ide_devices[drive].Model);
 
   return err;
}
#define ATA_FEATURES(x)     (x+1)
void EndianSwap(uint8_t *pData, int startIndex, int length)
{
    int i,cnt,end,start;
    cnt = length / 2;
    start = startIndex;
    end  = startIndex + length - 1;
    uint8_t tmp;
    for (i = 0; i < cnt; i++)
    {
        tmp            = pData[start+i];
        pData[start+i] = pData[end-i];
        pData[end-i]   = tmp;
    }
}
void read_atapi_capacity(atapi_capacity_t*info,ide_device *driver)
{
  	uint8_t read_cmd[12] = { 0x25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	uint8_t status;
	//int size;
   ide_write(driver->Channel, ATA_REG_HDDEVSEL, 0xA0 | (driver->Drive << 4)); // Select Drive.
   ksleep(1); // Wait 1ms for drive select to work.
   //outb(ATA_FEATURES(driver->Channel),0);//SET PIO MODE
   ide_write(driver->Channel,ATA_REG_FEATURES,0);
   ide_write(driver->Channel,ATA_REG_LBA1,0x08);
   ide_write(driver->Channel,ATA_REG_LBA2,0x08);
   ide_write(driver->Channel,ATA_REG_COMMAND,ATA_CMD_PACKET);
   uint8_t err;
   if(err=ide_polling(driver->Channel,1))return;

   asm("rep   outsw" : : "c"(6), "d"(channels[driver->Channel].base), "S"(read_cmd));
   //printf("wait for irq...");
   wait_for_irq();
   if(err=ide_polling(driver->Channel,1))return;
   //printf("ok!");
   int size=(ide_read(driver->Channel,ATA_REG_LBA2)<<8)|ide_read(driver->Channel,ATA_REG_LBA1);
   //ide_read_buffer(driver->Channel,ATA_REG_DATA,&cap,sizeof(atapi_capacity_t)/4);
   //
   insl(channels[driver->Channel].base,info,size/4);
   EndianSwap(info,0,4);
   EndianSwap(info,4,4);
   /**
    * @brief WHY USE THESE FUNCTIONS?(EndianSwap)
    * SO....
    * The wikipage(atapi from osdev) doesnt mention that...
    * THE FUCK return info is Large Endian.
    * (reserved...)
    */
   //printf("[%d %d %d]",size,info->max_lba,info->unit_size);
}
unsigned char ide_ata_access(unsigned char direction, unsigned char drive, unsigned int lba, 
unsigned char numsects,void *buffer)
{
   if(ide_devices[drive].UnitSize!=512)
   {
      //NOT SUPPORT UnitSize
      return 113;
   }
   unsigned char lba_mode /* 0: CHS, 1:LBA28, 2: LBA48 */, dma /* 0: No DMA, 1: DMA */, cmd;
   unsigned char lba_io[6];
   unsigned int  channel      = ide_devices[drive].Channel; // Read the Channel.
   unsigned int  slavebit      = ide_devices[drive].Drive; // Read the Drive [Master/Slave]
   unsigned int  bus = channels[channel].base; // Bus Base, like 0x1F0 which is also data port.
   unsigned int  words      = 256; // Almost every ATA drive has a sector-size of 512-byte.
   unsigned short cyl, i;
   unsigned char head, sect, err;
   uint16_t *target=buffer;
   // (I) Select one from LBA28, LBA48 or CHS;
   if (lba >= 0x10000000) { // Sure Drive should support LBA in this case, or you are
                            // giving a wrong LBA.
      // LBA48:
      lba_mode  = 2;
      lba_io[0] = (lba & 0x000000FF) >> 0;
      lba_io[1] = (lba & 0x0000FF00) >> 8;
      lba_io[2] = (lba & 0x00FF0000) >> 16;
      lba_io[3] = (lba & 0xFF000000) >> 24;
      lba_io[4] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
      lba_io[5] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
      head      = 0; // Lower 4-bits of HDDEVSEL are not used here.
   } else if (ide_devices[drive].Capabilities & 0x200)  { // Drive supports LBA?
      // LBA28:
      lba_mode  = 1;
      lba_io[0] = (lba & 0x00000FF) >> 0;
      lba_io[1] = (lba & 0x000FF00) >> 8;
      lba_io[2] = (lba & 0x0FF0000) >> 16;
      lba_io[3] = 0; // These Registers are not used here.
      lba_io[4] = 0; // These Registers are not used here.
      lba_io[5] = 0; // These Registers are not used here.
      head      = (lba & 0xF000000) >> 24;
   } else {
      // CHS:
      lba_mode=0;
      //However we dont support CHS mode
      //So we return 
      return 114;
   }
    // (II) See if drive supports DMA or not;
   dma = 0; // We don't support DMA
   // (III) Wait if the drive is busy;
   while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY){   
 
   } // Wait if busy.
   ide_write(channel, ATA_REG_HDDEVSEL, 0xE0 | (slavebit << 4) | head); // Drive & LBA
      // (V) Write Parameters;
   if (lba_mode == 2) {
      ide_write(channel, ATA_REG_SECCOUNT1,   0);
      ide_write(channel, ATA_REG_LBA3,   lba_io[3]);
      ide_write(channel, ATA_REG_LBA4,   lba_io[4]);
      ide_write(channel, ATA_REG_LBA5,   lba_io[5]);
   }
   ide_write(channel, ATA_REG_SECCOUNT0,   numsects);
   ide_write(channel, ATA_REG_LBA0,   lba_io[0]);
   ide_write(channel, ATA_REG_LBA1,   lba_io[1]);
   ide_write(channel, ATA_REG_LBA2,   lba_io[2]);
   if (lba_mode == 0 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;
   if (lba_mode == 1 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;   
   if (lba_mode == 2 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO_EXT;   
   if (lba_mode == 0 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA;
   if (lba_mode == 1 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA;
   if (lba_mode == 2 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA_EXT;
   if (lba_mode == 0 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
   if (lba_mode == 1 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
   if (lba_mode == 2 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO_EXT;
   if (lba_mode == 0 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA;
   if (lba_mode == 1 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA;
   if (lba_mode == 2 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA_EXT;
   ide_write(channel, ATA_REG_COMMAND, cmd);               // Send the Command.
      if (dma){
      if (direction == 0);
         // DMA Read.
      else;
         // DMA Write.
      }
   else{
      if (direction == 0)
      {
         // PIO Read.
      for (i = 0; i < numsects; i++) {
         if (err = ide_polling(channel, 1))
            return err; // Polling, set error and exit if there is.
         // asm("pushw %es");
         // asm("mov %%ax, %%es" : : "a"(selector));
         // asm("rep insw" : : "c"(words), "d"(bus), "D"(edi)); // Receive Data.
         // asm("popw %es");
         // edi += (words*2);
         insw(bus,target,512/2);
         target+=512/2;
      } 
      }
      else {
      // PIO Write.
         for (i = 0; i < numsects; i++) {
            ide_polling(channel, 0); // Polling.
            // asm("pushw %ds");
            // asm("mov %%ax, %%ds"::"a"(selector));
            // asm("rep outsw"::"c"(words), "d"(bus), "S"(edi)); // Send Data
            // asm("popw %ds");
            // edi += (words*2);
            outsw(bus,target,512/2);
            target+=512/2;
         }
         ide_write(channel, ATA_REG_COMMAND, (char []) {   ATA_CMD_CACHE_FLUSH,
                        ATA_CMD_CACHE_FLUSH,
                        ATA_CMD_CACHE_FLUSH_EXT}[lba_mode]);
         ide_polling(channel, 0); // Polling.
      }
   }
   return 0; // Easy, isn't it?
}
unsigned char ide_atapi_read(unsigned char drive, unsigned int lba, unsigned char numsects,
 void *buffer) {
   unsigned int   channel  = ide_devices[drive].Channel;
   unsigned int   slavebit = ide_devices[drive].Drive;
   unsigned int   bus      = channels[channel].base;
   //unsigned int   words    = 1024; // Sector Size. ATAPI drives have a sector size of 2048 bytes.
   unsigned char  err;
   int i;
   uint16_t *target=buffer;
   ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN = ide_irq_invoked = 0x0);
   // (I): Setup SCSI Packet:
   // ------------------------------------------------------------------
   atapi_packet[ 0] = ATAPI_CMD_READ;
   atapi_packet[ 1] = 0x0;
   atapi_packet[ 2] = (lba >> 24) & 0xFF;
   atapi_packet[ 3] = (lba >> 16) & 0xFF;
   atapi_packet[ 4] = (lba >> 8) & 0xFF;
   atapi_packet[ 5] = (lba >> 0) & 0xFF;
   atapi_packet[ 6] = 0x0;
   atapi_packet[ 7] = 0x0;
   atapi_packet[ 8] = 0x0;
   atapi_packet[ 9] = numsects;
   atapi_packet[10] = 0x0;
   atapi_packet[11] = 0x0;
   // (II): Select the drive:
   // ------------------------------------------------------------------
   ide_write(channel, ATA_REG_HDDEVSEL, slavebit << 4);
   // (III): Delay 400 nanoseconds for select to complete:
   // ------------------------------------------------------------------
   for(int i = 0; i < 4; i++)
       ide_read(channel, ATA_REG_ALTSTATUS); // Reading the Alternate Status port wastes 100ns.
      // (IV): Inform the Controller that we use PIO mode:
   // ------------------------------------------------------------------
   ide_write(channel, ATA_REG_FEATURES, 0);         // PIO mode.
   // (V): Tell the Controller the size of buffer:
   // ------------------------------------------------------------------
   ide_write(channel, ATA_REG_LBA1, (2048) & 0xFF);   // Lower Byte of Sector Size.
   ide_write(channel, ATA_REG_LBA2, (2048) >> 8);   // Upper Byte of Sector Size.
   // (VI): Send the Packet Command:
   // ------------------------------------------------------------------
   ide_write(channel, ATA_REG_COMMAND, ATA_CMD_PACKET);      // Send the Command.
 
   // (VII): Waiting for the driver to finish or return an error code:
   // ------------------------------------------------------------------
   if (err = ide_polling(channel, 1)) return err;         // Polling and return if error.
 
   // (VIII): Sending the packet data:
   // ------------------------------------------------------------------
   asm("rep   outsw" : : "c"(6), "d"(bus), "S"(atapi_packet));   // Send Packet Data
   // (IX): Receiving Data:
   // ------------------------------------------------------------------
   for (i = 0; i < numsects; i++) {
      wait_for_irq();                  // Wait for an IRQ.
      if (err = ide_polling(channel, 1))
         return err;      // Polling and return if error.
      // asm("pushw %es");
      // asm("mov %%ax, %%es"::"a"(selector));
      // asm("rep insw"::"c"(words), "d"(bus), "D"(edi));// Receive Data.
      // asm("popw %es");
      // edi += (words * 2);
      insw(bus,target,2048/2);
      target+=2048/2;
   }
      // (X): Waiting for an IRQ:
   // ------------------------------------------------------------------
   wait_for_irq();
 
   // (XI): Waiting for BSY & DRQ to clear:
   // ------------------------------------------------------------------
   while (ide_read(channel, ATA_REG_STATUS) & (ATA_SR_BSY | ATA_SR_DRQ))
      ;
   return 0; // Easy, ... Isn't it?
}            
int ide_read_sectors(unsigned char drive, unsigned char numsects, unsigned int lba,
                      uint8_t *buffer) {
 
   // ==================================
   if (drive > 3 || ide_devices[drive].Reserved == 0) 
   {
      printf("Drive %d Not Found!\n",drive);
      return -1;      // Drive Not Found!
   }
   
   // 2: Check if inputs are valid:
   // ==================================
                    // Seeking to invalid position.
 
   // 3: Read in PIO Mode through Polling & IRQs:
   // ============================================
   else {
      unsigned char err;
      if (ide_devices[drive].Type == IDE_ATA)
      {
         err = ide_ata_access(ATA_READ, drive, lba, numsects,buffer);
      }
      else if (ide_devices[drive].Type == IDE_ATAPI)
         for (int i = 0; i < numsects; i++)
         {
            err = ide_atapi_read(drive, lba + i, 1, buffer);
            buffer+=2048;
         }
      return ide_print_error(drive, err);
   }
}  

int ide_dev_read(kdevice_t*self, uint32_t addr,uint32_t num,char *buffer,int flag)
{
   //_IO_ATOMIC_IN
   lock_acquire(&ata_lock);
   int ret= ide_read_sectors(self->dev_id,num,addr,buffer);
   //_IO_ATOMIC_OUT
   lock_release(&ata_lock);
   return ret;
}
int ide_dev_write(kdevice_t*self, uint32_t addr,uint32_t num,char *buffer,int flag)
{
   if(self->type==KDEV_ATAPI)return -4;
   //_IO_ATOMIC_IN
   lock_acquire(&ata_lock);
   int ret= ide_ata_access(ATA_WRITE,self->dev_id,addr,num,buffer);
   //_IO_ATOMIC_OUT
   lock_release(&ata_lock);
   return ret;
   //if(self->type==KDEV_ATAPI)return ide_atapi_read(self->dev_id,addr,num,buffer);
}
char *zeros=0;
int ide_dev_zero(struct kdevice *self,uint32_t addr,uint32_t num,uint32_t flag)
{
   printf("zeros self:%x",self);
   while (num--)
   {
      ide_ata_access(ATA_WRITE,self->dev_id,addr,1,zeros);
      addr++;
   }
   return 1;
   
}
kdevice_ops_t ata_dev_ops={
   .read=ide_dev_read,
   .write=ide_dev_write,
   .zeros=ide_dev_zero
};
void ide_initialize(unsigned int BAR0, unsigned int BAR1, unsigned int BAR2, unsigned int BAR3,
unsigned int BAR4) {
   lock_init(&ata_lock);
   zeros=kmalloc(512);
   if(zeros)
   {
      memset(zeros,0,512);
   }
   int j, k, count = 0;
   register_interrupt_handler(IRQ14,ide_irq);
   register_interrupt_handler(IRQ15,ide_irq);
   // 1- Detect I/O Ports which interface IDE Controller:
   channels[ATA_PRIMARY  ].base  = (BAR0 & 0xFFFFFFFC) + 0x1F0 * (!BAR0);
   channels[ATA_PRIMARY  ].ctrl  = (BAR1 & 0xFFFFFFFC) + 0x3F6 * (!BAR1);
   channels[ATA_SECONDARY].base  = (BAR2 & 0xFFFFFFFC) + 0x170 * (!BAR2);
   channels[ATA_SECONDARY].ctrl  = (BAR3 & 0xFFFFFFFC) + 0x376 * (!BAR3);
   channels[ATA_PRIMARY  ].bmide = (BAR4 & 0xFFFFFFFC) + 0; // Bus Master IDE
   channels[ATA_SECONDARY].bmide = (BAR4 & 0xFFFFFFFC) + 8; // Bus Master IDE
    // 2- Disable IRQs:
   ide_write(ATA_PRIMARY  , ATA_REG_CONTROL, 2);
   ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);
   // 3- Detect ATA-ATAPI Devices:
   for (int i = 0; i < 2; i++)
      for (j = 0; j < 2; j++) {
 
         unsigned char err = 0, type = IDE_ATA, status;
         ide_devices[count].Reserved = 0; // Assuming that no drive here.
 
         // (I) Select Drive:
         ide_write(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4)); // Select Drive.
         ksleep(1); // Wait 1ms for drive select to work.
 
         // (II) Send ATA Identify Command:
         ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
         ksleep(1); // This function should be implemented in your OS. which waits for 1 ms.
                   // it is based on System Timer Device Driver.
 
         // (III) Polling:
         //printf("a");
         if (ide_read(i, ATA_REG_STATUS) == 0) continue; // If Status = 0, No Device.
        //printf("b");
        reset_tick();
         while(1) {
            if(get_tick()>10000){err=1;break;}
            status = ide_read(i, ATA_REG_STATUS);
            if ((status & ATA_SR_ERR)) {err = 1; break;} // If Err, Device is not ATA.
            if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ))
            {
                // if(inb(0x1f4)||inb(0x1f5))
                // {
                //     err==1;
                //     break;
                // }
                break; // Everything is right.
            } 
         }
        //printf("c");
         // (IV) Probe for ATAPI Devices:
 
         if (err != 0) {
            //printf("c1");
            unsigned char cl = ide_read(i, ATA_REG_LBA1);
            unsigned char ch = ide_read(i, ATA_REG_LBA2);
 
            if (cl == 0x14 && ch ==0xEB)
               type = IDE_ATAPI;
            else if (cl == 0x69 && ch == 0x96)
               type = IDE_ATAPI;
            //else if(cl||ch)
            //    type==IDE_ATAPI;
            else
               continue; // Unknown Type (may not be a device).
 
            ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
            
            ksleep(1);
         }
 
         // (V) Read Identification Space of the Device:
         ide_read_buffer(i, ATA_REG_DATA, (unsigned int) ide_buf, 128);
 
         // (VI) Read Device Parameters:
         ide_devices[count].Reserved     = 1;
         ide_devices[count].Type         = type;
         ide_devices[count].Channel      = i;
         ide_devices[count].Drive        = j;
         ide_devices[count].Signature    = *((unsigned short *)(ide_buf + ATA_IDENT_DEVICETYPE));
         ide_devices[count].Capabilities = *((unsigned short *)(ide_buf + ATA_IDENT_CAPABILITIES));
         ide_devices[count].CommandSets  = *((unsigned int *)(ide_buf + ATA_IDENT_COMMANDSETS));
 
         // (VII) Get Size:
         bool lba_48=ide_devices[count].CommandSets & (1 << 26);
         //ATAPI_IDENTIFY_DATA *dd=ide_buf;
         //printf("???%d+%d???",dd+dd->lba28Sectors[1],dd->lba48Sectors[0]+dd->lba48Sectors[1]+dd->lba48Sectors[2]+dd->lba48Sectors[3]);
         ide_devices[count].UnitSize=512;
         if (lba_48)
         {
            // Device uses 48-Bit Addressing:
            ide_devices[count].MaxLBA   = (*((unsigned int *)(ide_buf + ATA_IDENT_MAX_LBA_EXT)));
         }
         else
         {
            // Device uses CHS or 28-bit Addressing:
            ide_devices[count].MaxLBA   = (*((unsigned int *)(ide_buf + ATA_IDENT_MAX_LBA)));
         }
        //This is mothod 1
        /**
         * @brief NOTE
         * I found this method doesnt work at all
         * so, i choose another method to replace it
         */
        /*if(!ide_devices[count].Size)
        {
            ide_write(i,ATA_REG_COMMAND,0x27);   
            while (ide_read(i,ATA_REG_COMMAND)&0x80!=0)
            {
                
            }
            //METHOD 2
            uint32_t mx_lba=0;
            if(lba_48)
            {
               mx_lba=(uint64_t)ide_read(i,3);
               mx_lba+=(uint64_t)ide_read(i,4)<<8;
               mx_lba+=(uint64_t)ide_read(i,5)<<16;
               ide_write(i,2,0x80);
               mx_lba=(uint64_t)ide_read(i,3)<<24;
               mx_lba+=(uint64_t)ide_read(i,4)<<32;
               mx_lba+=(uint64_t)ide_read(i,5)<<40;
            }else
            {
               mx_lba=(uint64_t)ide_read(i,3);
               mx_lba+=(uint64_t)ide_read(i,4)<<8;
               mx_lba+=(uint64_t)ide_read(i,5)<<16;
               mx_lba+=((uint64_t)ide_read(i,6)&0xf)<<24;
            }
            if(!mx_lba)
            {
               printf("FAIL TO GET SIZE!");
            }
            ide_devices[count].Size=mx_lba;
            
        }*/
         // (VIII) String indicates model of device (like Western Digital HDD and SONY DVD-RW...):
         for(k = 0; k < 40; k += 2) {
            ide_devices[count].Model[k] = ide_buf[ATA_IDENT_MODEL + k + 1];
            ide_devices[count].Model[k + 1] = ide_buf[ATA_IDENT_MODEL + k];}
         ide_devices[count].Model[40] = 0; // Terminate String.
 
         count++;
         //printf("pok");
      }
 
   // 4- Print Summary:
   //printf("ok!");
   for (int i = 0; i < 4; i++)
      if (ide_devices[i].Reserved == 1) {
         if(ide_devices[i].MaxLBA==0&&ide_devices[i].Type==1)
         {
            atapi_capacity_t cap;
            //printf("ATTENTION Ready to read capacity from ATAPI!\n");
            read_atapi_capacity(&cap,&ide_devices[i]);
            if(!cap.max_lba||!cap.unit_size)
            {
               printf("\nUnknow Error: Cannot get capacity of the atapi device!\n");
               while(1);
            }
            
            ide_devices[i].MaxLBA=cap.max_lba;
            ide_devices[i].UnitSize= cap.unit_size;
         }
         printf(" Found %s Drive %dByte - %s\n",
            (const char *[]){"ATA", "ATAPI"}[ide_devices[i].Type],         /* Type */
            ide_devices[i].MaxLBA*ide_devices[i].UnitSize ,               /* Size */
            ide_devices[i].Model);
         char buf[20]={0};
         sprintf(buf,"ata%d",i);
         if(ide_devices[i].Type)
         {
            device_add(device_create(buf,KDEV_BLOCK,KDEV_ATAPI,i,0,2048,&ata_dev_ops,0,0));
         }else
         {
            device_add(device_create(buf,KDEV_BLOCK,KDEV_ATA,i,0,512,&ata_dev_ops,0,0));
         }
         
      }
}