#define XA_LENGTH(x) (((u64)(x) * 75) / 100 * IO_SECT_SIZE) //Centiseconds to sectors in bytes (w)

typedef struct
{
	XA_File file;
	u32 length;
} XA_TrackDef;

static const XA_TrackDef xa_tracks[] = {
	//MENU.XA
	{XA_Menu, XA_LENGTH(12200)}, //XA_GettinFreaky
	{XA_Menu, XA_LENGTH(6500)},  //XA_GameOver
	//WEEK1A.XA
	{XA_Week1A, XA_LENGTH(8550)}, //XA_Bopeebo
	{XA_Week1A, XA_LENGTH(10000)}, //XA_Fresh
	//WEEK1B.XA
	{XA_Week1B, XA_LENGTH(11000)}, //XA_Dadbattle
	{XA_Week1B, XA_LENGTH(5140)}, //XA_Tutorial
	//WEEK2A.XA
	{XA_Week2A, XA_LENGTH(10500)}, //XA_Spookeez
	{XA_Week2A, XA_LENGTH(8900)}, //XA_South
	//WEEK2B.XA
	{XA_Week2B, XA_LENGTH(10000)}, //XA_Monster
	{XA_Week2B, XA_LENGTH(14200)}, //XA_Clucked
	//WEEK3A.XA
	{XA_Week3A, XA_LENGTH(8400)},  //XA_Pico
	{XA_Week3A, XA_LENGTH(10500)}, //XA_Philly
	//WEEK3B.XA
	{XA_Week3B, XA_LENGTH(10200)}, //XA_Blammed
	//WEEK4A.XA
	{XA_Week4A, XA_LENGTH(10400)},  //XA_SatinPanties
	{XA_Week4A, XA_LENGTH(12800)}, //XA_High
	//WEEK4B.XA
	{XA_Week4B, XA_LENGTH(11600)}, //XA_Awooga
	{XA_Week4B, XA_LENGTH(11600)}, //XA_MILF

	{XA_Week1S, XA_LENGTH(200)},  //XA_GameOver
};

static const char *xa_paths[] = {
	"\\MUSIC\\MENU.XA;1",   //XA_Menu
	"\\MUSIC\\WEEK1A.XA;1", //XA_Week1A
	"\\MUSIC\\WEEK1B.XA;1", //XA_Week1B
	"\\MUSIC\\WEEK2A.XA;1", //XA_Week2A
	"\\MUSIC\\WEEK2B.XA;1", //XA_Week2B
	"\\MUSIC\\WEEK3A.XA;1", //XA_Week3A
	"\\MUSIC\\WEEK3B.XA;1", //XA_Week3B
	"\\MUSIC\\WEEK4A.XA;1", //XA_Week4A
	"\\MUSIC\\WEEK4B.XA;1", //XA_Week4B
	"\\MUSIC\\WEEK1S.XA;1", //XA_Week4B
	NULL,
};

typedef struct
{
	const char *name;
	boolean vocal;
} XA_Mp3;

static const XA_Mp3 xa_mp3s[] = {
	//MENU.XA
	{"freaky", false},   //XA_GettinFreaky
	{"gameover", false}, //XA_GameOver
	//WEEK1A.XA
	{"bopeebo", true}, //XA_Bopeebo
	{"fresh", true},   //XA_Fresh
	//WEEK1B.XA
	{"dadbattle", true}, //XA_Dadbattle
	{"tutorial", false}, //XA_Tutorial
	//WEEK2A.XA
	{"spookeez", true}, //XA_Spookeez
	{"south", true},    //XA_South
	//WEEK2B.XA
	{"ghastly", true}, //XA_Ghastly
	{"monster", true}, //XA_Monster
	//WEEK3A.XA
	{"pico", true},   //XA_Pico
	{"philly", true}, //XA_Philly
	//WEEK3B.XA
	{"blammed", true}, //XA_Blammed
	//WEEK4A.XA
	{"satinpanties", true}, //XA_SatinPanties
	{"high", true},         //XA_High
	//WEEK4B.XA
	{"awooga", true}, //XA_Awooga
	{"milf", true}, //XA_MILF

	{"lights", false}, //XA_GameOver
	//WEEK5A.XA
	{"cocoa", true},  //XA_Cocoa
	{"eggnog", true}, //XA_Eggnog
	//WEEK5B.XA
	{"winterhorrorland", true}, //XA_WinterHorrorland
	//WEEK6A.XA
	{"senpai", true}, //XA_Senpai
	{"roses", true},  //XA_Roses
	//WEEK6B.XA
	{"thorns", true}, //XA_Thorns
	//WEEK1S.XA
	//WEEK7A.XA
	{"ugh", true},  //XA_Ugh
	{"guns", true}, //XA_Guns
	//WEEK7B.XA
	{"stress", true}, //XA_Stress
	//KAPIA.XA
	{"wocky", true},     //XA_Wocky
	{"beathoven", true}, //XA_Beathoven
	//KAPIB.XA
	{"hairball", true}, //XA_Hairball
	{"nyaw", true},     //XA_Nyaw
	//CLWNA.XA
	{"improbableoutset", true}, //XA_ImprobableOutset
	{"madness", true}, //XA_Madness
	//CLWNB.XA
	{"hellclown", true}, //XA_Hellclown
	{"expurgation", true}, //XA_Expurgation
	
	{NULL, false}
};
