#include "BaseEvent.h"

BaseEvent::BaseEvent()
{
	address = 0;
	raw = 0;
}

void BaseEvent::start()
{
	raw = PVZ::Memory::ReadMemory<BYTE>(address);
	PVZ::Memory::WriteMemory<BYTE>(address, 0xCC);
}

void BaseEvent::end()
{
	PVZ::Memory::WriteMemory<BYTE>(address, raw);
}

void BaseEvent::handle(CONTEXT& context)
{
	std::cout << "������BaseEvent��handle()���ⲻӦ�÷�����\n";
}