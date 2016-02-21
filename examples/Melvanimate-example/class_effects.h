// test class effects....


#pragma once



class testclass : public EffectHandler, public Color_property, public Brightness_property, public Palette_property
{

public:
  testclass(): Color_property(this), Brightness_property(this), Palette_property(this) {};


  bool Run() override {};
  bool Start() override {};
  bool Stop() override {};
  void Refresh() override {}; 

  // bool addEffectJson(JsonObject& settings) override
  // {
  //  Serial.printf("[CascadeEffect::addJson] Called\n");
  //  Serial.println("[CascadeEffect::addJson] root");
  //  settings.prettyPrintTo(Serial);
  //  Serial.println();
  // }

//  bool parseJsonEffect(JsonObject& root) override;

  // bool testFn()
  // {
  //  _color = RgbColor(0, 0, 0);
  //  _brightness = 255;
  // }
};

