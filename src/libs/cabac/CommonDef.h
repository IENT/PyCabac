#ifndef COMMONDEF_H
#define COMMONDEF_H

#include <cstring>
#include <utility>
#include <sstream>

// We assume that 1000 contexts are enough for most of the scenarios.
#define RWTH_PYTHON_IF 1
#define RWTH_ENABLE_TRACING 1

class Exception : public std::exception
{
public:
  Exception( const std::string& _s ) : m_str( _s ) { }
  Exception( const Exception& _e ) : std::exception( _e ), m_str( _e.m_str ) { }
  virtual ~Exception() noexcept { };
  virtual const char* what() const noexcept { return m_str.c_str(); }
  Exception& operator=( const Exception& _e ) { std::exception::operator=( _e ); m_str = _e.m_str; return *this; }
  template<typename T> Exception& operator<<( T t ) { std::ostringstream oss; oss << t; m_str += oss.str(); return *this; }
private:
  std::string m_str;
};

// if a check fails with THROW or CHECK, please check if ported correctly from assert in revision 1196)
#define THROW(x)            throw( Exception( "\nERROR: In function \"" ) << __FUNCTION__ << "\" in " << __FILE__ << ":" << __LINE__ << ": " << x )
#define CHECK(c,x)          if(c){ THROW(x); }
#define EXIT(x)             throw( Exception( "\n" ) << x << "\n" )
#define CHECK_NULLPTR(_ptr) CHECK( !( _ptr ), "Accessing an empty pointer pointer!" )
#define CHECKD(c,x)


// from CommonDef.h
static constexpr int SCALE_BITS = 15;  // Precision for fractional bit estimates

static const int RExt__GOLOMB_RICE_ADAPTATION_STATISTICS_SETS =     4;



#endif /* COMMONDEF_H */
