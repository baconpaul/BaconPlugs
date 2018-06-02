# Testing with the "Standalone" directory

There's a standalone directory which runs without rack for unit tests and demos. To use it, you have to get your
plugin code out of rack-land in headers so you can include it in a non-rack context.

The way I do that basically is

* Write my steppers so they are not subclasses of module but rather are parameterized by their base class
   * This means you need to add a using TBase::params and so on to get the name resolution working
   * And I generally typedef so in FooWidget I have typedef Foo< Module > M and then use M
* In standalone, include it and have a stubbed-out base class which gives me vectors I can read and write from
  without the runtime
  
All pretty clear. Look at the SampleDelay for a cleanly worked example.
