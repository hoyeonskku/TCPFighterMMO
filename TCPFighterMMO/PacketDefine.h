#pragma once
#define dfERROR_RANGE		50
#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		10
#define dfATTACK2_RANGE_Y		10
#define dfATTACK3_RANGE_Y		20

//---------------------------------------------------------------
// ��Ŷ���.
//
//---------------------------------------------------------------
#pragma pack(push, 1)
struct PacketHeader
{
	unsigned char byCode = 0x89;			    // ��Ŷ�ڵ� 0x89 ����.
	unsigned char bySize;						// ��Ŷ ������.
	unsigned char byType;						// ��ŶŸ��.
};

#define	dfPACKET_SC_CREATE_MY_CHARACTER			0
//---------------------------------------------------------------
// Ŭ���̾�Ʈ �ڽ��� ĳ���� �Ҵ�		Server -> Client
//
// ������ ���ӽ� ���ʷ� �ްԵǴ� ��Ŷ���� �ڽ��� �Ҵ���� ID ��
// �ڽ��� ���� ��ġ, HP �� �ް� �ȴ�. (ó���� �ѹ� �ް� ��)
// 
// �� ��Ŷ�� ������ �ڽ��� ID,X,Y,HP �� �����ϰ� ĳ���͸� �������Ѿ� �Ѵ�.
//---------------------------------------------------------------

struct PACKET_SC_CREATE_MY_CHARACTER
{
	unsigned int id;					//	4	-	ID
	unsigned char Direction;	//	1	-	Direction	(LL / RR)
	unsigned short x;					//	2	-	X
	unsigned short y;					//	2	-	Y
	unsigned char hp;					//	1	-	HP
};

#define	dfPACKET_SC_CREATE_OTHER_CHARACTER		1
//---------------------------------------------------------------
// �ٸ� Ŭ���̾�Ʈ�� ĳ���� ���� ��Ŷ		Server -> Client
//
// ó�� ������ ���ӽ� �̹� ���ӵǾ� �ִ� ĳ���͵��� ����
// �Ǵ� �����߿� ���ӵ� Ŭ���̾�Ʈ���� ������ ����.
//---------------------------------------------------------------

struct PACKET_SC_CREATE_OTHER_CHARACTER
{
	unsigned int id;					//	4	-	ID
	unsigned char Direction;	//	1	-	Direction	(LL / RR)
	unsigned short x;					//	2	-	X
	unsigned short y;					//	2	-	Y
	unsigned char hp;					//	1	-	HP
};

#define	dfPACKET_SC_DELETE_CHARACTER			2
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ						Server -> Client
//
// ĳ������ �������� �Ǵ� ĳ���Ͱ� �׾����� ���۵�.
//
//	4	-	ID
//
//---------------------------------------------------------------

struct PACKET_SC_DELETE_CHARACTER
{
	unsigned int id;					//	4	-	ID
};


#define	dfPACKET_CS_MOVE_START					10
//---------------------------------------------------------------
// ĳ���� �̵����� ��Ŷ						Client -> Server
//
// �ڽ��� ĳ���� �̵����۽� �� ��Ŷ�� ������.
// �̵� �߿��� �� ��Ŷ�� ������ ������, Ű �Է��� ����Ǿ��� ��쿡��
// ������� �Ѵ�.
//
// (���� �̵��� ���� �̵� / ���� �̵��� ���� ���� �̵�... ���)
//---------------------------------------------------------------

struct PACKET_CS_MOVE_START
{
	unsigned char Direction;	//	1	-	Direction
	unsigned short x;					//	2	-	X
	unsigned short y;					//	2	-	Y
};

#define dfPACKET_MOVE_DIR_LL					0
#define dfPACKET_MOVE_DIR_LU					1
#define dfPACKET_MOVE_DIR_UU					2
#define dfPACKET_MOVE_DIR_RU					3
#define dfPACKET_MOVE_DIR_RR					4
#define dfPACKET_MOVE_DIR_RD					5
#define dfPACKET_MOVE_DIR_DD					6
#define dfPACKET_MOVE_DIR_LD					7

#define	dfPACKET_SC_MOVE_START					11
//---------------------------------------------------------------
// ĳ���� �̵����� ��Ŷ						Server -> Client
//
// �ٸ� ������ ĳ���� �̵��� �� ��Ŷ�� �޴´�.
// ��Ŷ ���Ž� �ش� ĳ���͸� ã�� �̵�ó���� ���ֵ��� �Ѵ�.
// 
// ��Ŷ ���� �� �ش� Ű�� ����ؼ� ���������� �����ϰ�
// �ش� �������� ��� �̵��� �ϰ� �־�߸� �Ѵ�.
//---------------------------------------------------------------

struct PACKET_SC_MOVE_START
{
	unsigned int id;					//	4	-	ID
	unsigned char Direction;	//	1	-	Direction
	unsigned short x;					//	2	-	X
	unsigned short y;					//	2	-	Y
};

#define	dfPACKET_CS_MOVE_STOP					12
//---------------------------------------------------------------
// ĳ���� �̵����� ��Ŷ						Client -> Server
//
// �̵��� Ű���� �Է��� ��� �����Ǿ��� ��, �� ��Ŷ�� ������ �����ش�.
// �̵��� ���� ��ȯ�ÿ��� ��ž�� ������ �ʴ´�.
//---------------------------------------------------------------
struct PACKET_CS_MOVE_STOP
{
	unsigned char Direction;	//	1	-	Direction	(LL / RR)
	unsigned short x;					//	2	-	X
	unsigned short y;					//	2	-	Y
};

#define	dfPACKET_SC_MOVE_STOP					13
//---------------------------------------------------------------
// ĳ���� �̵����� ��Ŷ						Server -> Client
//
// ID �� �ش��ϴ� ĳ���Ͱ� �̵��� ������̹Ƿ� 
// ĳ���͸� ã�Ƽ� �����, ��ǥ�� �Է����ְ� ���ߵ��� ó���Ѵ�.
//---------------------------------------------------------------
struct PACKET_SC_MOVE_STOP
{
	unsigned int id;					//	4	-	ID
	unsigned char Direction;	//	1	-	Direction	(LL / RR)
	unsigned short x;					//	2	-	X
	unsigned short y;					//	2	-	Y
};


#define	dfPACKET_CS_ATTACK1						20
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Client -> Server
//
// ���� Ű �Է½� �� ��Ŷ�� �������� ������.
// �浹 �� �������� ���� ����� �������� �˷� �� ���̴�.
//
// ���� ���� ���۽� �ѹ��� �������� ������� �Ѵ�.
//---------------------------------------------------------------
struct PACKET_CS_ATTACK1
{
	unsigned char Direction;	//	1	-	Direction	(LL / RR)
	unsigned short x;					//	2	-	X
	unsigned short y;					//	2	-	Y
};

#define	dfPACKET_SC_ATTACK1						21
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Server -> Client
//
// ��Ŷ ���Ž� �ش� ĳ���͸� ã�Ƽ� ����1�� �������� �׼��� �����ش�.
// ������ �ٸ� ��쿡�� �ش� �������� �ٲ� �� ���ش�.
//---------------------------------------------------------------
struct PACKET_SC_ATTACK1
{
	unsigned int id;					//	4	-	ID
	unsigned char Direction;	//	1	-	Direction	(LL / RR)
	unsigned short x;					//	2	-	X
	unsigned short y;					//	2	-	Y
};



#define	dfPACKET_CS_ATTACK2						22
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Client -> Server
//
// ���� Ű �Է½� �� ��Ŷ�� �������� ������.
// �浹 �� �������� ���� ����� �������� �˷� �� ���̴�.
//
// ���� ���� ���۽� �ѹ��� �������� ������� �Ѵ�.
//
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
struct PACKET_CS_ATTACK2
{
	unsigned char Direction;	//	1	-	Direction	(LL / RR)
	unsigned short x;					//	2	-	X
	unsigned short y;					//	2	-	Y
};

#define	dfPACKET_SC_ATTACK2						23
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Server -> Client
//
// ��Ŷ ���Ž� �ش� ĳ���͸� ã�Ƽ� ����2�� �������� �׼��� �����ش�.
// ������ �ٸ� ��쿡�� �ش� �������� �ٲ� �� ���ش�.
//
//	4	-	ID
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
struct PACKET_SC_ATTACK2
{
	unsigned int id;					//	4	-	ID
	unsigned char Direction;	//	1	-	Direction	(LL / RR)
	unsigned short x;					//	2	-	X
	unsigned short y;					//	2	-	Y
};
#define	dfPACKET_CS_ATTACK3						24
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Client -> Server
//
// ���� Ű �Է½� �� ��Ŷ�� �������� ������.
// �浹 �� �������� ���� ����� �������� �˷� �� ���̴�.
//
// ���� ���� ���۽� �ѹ��� �������� ������� �Ѵ�.
//
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
struct PACKET_CS_ATTACK3
{
	unsigned char Direction;	//	1	-	Direction	(LL / RR)
	unsigned short x;					//	2	-	X
	unsigned short y;					//	2	-	Y
};

#define	dfPACKET_SC_ATTACK3						25
//---------------------------------------------------------------
// ĳ���� ���� ��Ŷ							Server -> Client
//
// ��Ŷ ���Ž� �ش� ĳ���͸� ã�Ƽ� ����3�� �������� �׼��� �����ش�.
// ������ �ٸ� ��쿡�� �ش� �������� �ٲ� �� ���ش�.
//
//	4	-	ID
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
struct PACKET_SC_ATTACK3
{
	unsigned int id;					//	4	-	ID
	unsigned char Direction;	//	1	-	Direction	(LL / RR)
	unsigned short x;					//	2	-	X
	unsigned short y;					//	2	-	Y
};




#define	dfPACKET_SC_DAMAGE						30
//---------------------------------------------------------------
// ĳ���� ������ ��Ŷ							Server -> Client
//
// ���ݿ� ���� ĳ������ ������ ����.
//

//
//---------------------------------------------------------------
struct PACKET_SC_DAMAGE
{
	unsigned int AttackID;			//	4	-	AttackID	( ������ ID )
	unsigned int DamageID;			//	4	-	DamageID	( ������ ID )
	unsigned char DamageHP;			//	1	-	DamageHP	( ������ HP )
};

// ������...
#define	dfPACKET_CS_SYNC						250
//---------------------------------------------------------------
// ����ȭ�� ���� ��Ŷ					Client -> Server
//
//
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------


#define	dfPACKET_SC_SYNC						251
//---------------------------------------------------------------
// ����ȭ�� ���� ��Ŷ					Server -> Client
//
// �����κ��� ����ȭ ��Ŷ�� ������ �ش� ĳ���͸� ã�Ƽ�
// ĳ���� ��ǥ�� �������ش�.
//
//	4	-	ID
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
// 
// 
#pragma pack(pop)