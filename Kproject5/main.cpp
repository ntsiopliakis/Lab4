#include "gpiocregisters.hpp" //for GPIOC
#include "gpioaregisters.hpp" //for GPIOA
#include "rccregisters.hpp"   //for RCC
#include <iostream>

template<typename T> 
struct Port
{
  using PortType = T;

  static void Enable()
  {
    
  }
    static void Disable()
  {
    
  }
  static void Set(std::uint32_t value)
  {
    T::BSSR::Set(value);
  }

  static void Reset(std::uint32_t value)
  {
    T::BSSR::Set(value << 16U);
  }

  static void Toggle(std::uint32_t value)
  {
    T::ODR::Toggle(value);
  }

  static auto Get()
  {
    return T::IDR::Get();
  }

};

template< typename Port, uint8_t pinNum>
struct Pin
{
  using PortType = typename Port::PortType;
  static void Set()
  {
    Port::Set(1U << pinNum);
  }

  static void Reset()
  {
    Port::Reset(1U << pinNum);
  }

  static void Toggle()
  {
    Port::Toggle(1U << pinNum);
  }

  static auto IsSet()
  {
    return ((Port::Get() & (1U << pinNum)) != 0 );
  }
  static void SelectOutputMode()
  {
    if(pinNum<16U)
    {
      *reinterpret_cast<uint32_t*>(PortType::MODER::Address) &= ~(3U<<(pinNum*2));
      *reinterpret_cast<uint32_t*>(PortType::MODER::Address) |= 1U<<(pinNum*2); 
    }
  }
  static void SelectInputMode()
  {
    if(pinNum<16U)
    {
      *reinterpret_cast<uint32_t*>(PortType::MODER::Address) &= ~(3U<<(pinNum*2));
    }
  }
};

template<typename Pin>
struct Button
{  
  static bool WasPressed()
  {
    if(!Pin::IsSet())
    {
     for(int i = 0; i < 100; i++);
     if(!Pin::IsSet())
     {
       while(!Pin::IsSet());
       return true;
     }
    }
    return false;
  }
  static void Enable()
  {
    Pin::SelectInputMode();
  }
};

template<typename ... PinArgs>
struct Leds
{
  static void Toggle()
  {
    (PinArgs::Toggle(),...);
  }
  static void Enable()
  {
    (PinArgs::SelectOutputMode(),...);
  }
};

template<>
void Port<GPIOC>::Enable()
{
  RCC::AHB1ENR::GPIOCEN::Enable::Set();
}
template<>
 void Port<GPIOA>::Enable()
{
  RCC::AHB1ENR::GPIOAEN::Enable::Set();
}
template<>
void Port<GPIOC>::Disable()
{
  RCC::AHB1ENR::GPIOCEN::Disable::Set();
}
template<>
void Port<GPIOA>::Disable()
{
  RCC::AHB1ENR::GPIOAEN::Disable::Set();
}

int main()
{
  while(RCC::CR::HSIRDY::NotReady::IsSet());
  RCC::PLLCFGR::PLLN0::Set(125);
  RCC::PLLCFGR::PLLP0::Set(3);
  RCC::PLLCFGR::PLLM0::Set(20);                       
  RCC::CR::PLLON::On::Set();
  while(RCC::CR::PLLRDY::NotReady::IsSet());
  RCC::CFGR::SW::Pll::Set();
  while(!RCC::CFGR::SWS::Pll::IsSet());
  
  
  using PortC = Port<GPIOC>;
  using PortA = Port<GPIOA>;
  PortC::Enable();
  PortA::Enable();

  using LED1pin = Pin<PortC, 5>;
  using LED2pin = Pin<PortC, 8>;
  using LED3pin = Pin<PortC, 9>;
  using LED4pin = Pin<PortA, 5>;
  using UserLeds = Leds<LED1pin, LED2pin, LED3pin, LED4pin>;
  UserLeds::Enable();
  
  using UserButton = Button< Pin< PortC, 13> >;
  UserButton::Enable();

  while(1)
  {
    if(UserButton::WasPressed())
    {
      UserLeds::Toggle();
    }
  }
}