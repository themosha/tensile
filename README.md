# Tensile (tensile test for SQL)

Tensile is the tool which attempts to discover actual limits in SQL.
Tensile operates with two major concepts:
* SQL feature: This is a construct in SQL that we want to find maximum supported value of.
  Examples of SQL feature can be length of identifier, number of nested subselects etc.
* SQL provider: This is the component which supports SQL.
  It can be anything from SQL parser library to full blown database system.

Given a feature and provider, Tensile uses binary search to construct SQL statements using the feature of different sizes.
It then passes the SQL to the provider, and detects one of the following outcomes:
* Success - Provider handled SQL
* Error - Provider failed gracefully
* Timeout - Provider took longer than specified timeout to handle SQL
* Crash - Provider crashed while processing given SQL

## Running Tensile
Tensile comes with builtin list of features to check, and two sample providers - Hyrise parser and Hyrise executor.
By default it runs every builtin feature against both providers. Running it would look like following:

```
> tensile
parenthesis:........E.EEEEE. limit = 193 status = Error
string literal:......................T...T.T..TT.T.TTTTTTT. limit = 4023553 status = Timeout
array:......................T..T.TT.TTTTTTTTTTTTTT limit = 3817472 status = Timeout
nested array:........E.EEEEE. limit = 193 status = Error
tuple:......................T..TT...T..T.TT...T.T. limit = 3791477 status = Timeout
nested tuple:.......EEEEEE. limit = 65 status = Error
select list:................T............... limit = 65535 status = Timeout
unary operator - :........E.EEEE.E limit = 194 status = Error
unary operator NOT :# limit = 1 status = Crash
binary operator +:.....TT.TT limit = 20 status = Timeout
binary operator -:.....TT.TT limit = 20 status = Timeout
binary operator  AND :.............#.#######..#. limit = 6157 status = Crash
binary operator  || :..........T.TT...... limit = 831 status = Timeout
function abs():.......EEEEEEE limit = 64 status = Error
function trim():.......EEEEEEE limit = 64 status = Error
union all:.......EE..... limit = 95 status = Error
cross join:..........TTT.T.TTT. limit = 593 status = Timeout
natural join:.......T.T..TT limit = 108 status = Timeout
chain join:.......TT.T..T limit = 86 status = Timeout
star join:.......TT.T..T limit = 86 status = Timeout
in semijoin:.....EE.EE limit = 20 status = Error
exists semijoin:.....EE... limit = 23 status = Error
```

## Adding providers
In order to check limits of your SQL system, add a class deriving from

```
class ISQLProvider {
 public:
  // Human readable name
  virtual std::string name() const = 0;
  // Take given SQL query, and run it (can be parse, analyze, execute etc)
  virtual bool Run(const std::string& sql) = 0;
};
```
and it order for Tensile to run it automatically, make sure to call `RegisterSQLProvider` before `main`.
Example how to do it can be found in `providers\hyrise.cpp`.

## Adding features
It is also possible to extend Tensile by adding new SQL features. They should derive from

```
class ISQLFeature {
 public:
  // Human readable name of the feature
  virtual std::string name() = 0;
  // Generates valid SQL query which has feature with given cardinality @n
  virtual std::string GenerateSQL(size_t n) = 0;
  // Test only method for derived class instance to be able test 
  // that it generates desired SQL. This will usually call GenerateSQL
  // for low values of @n = 1,2,... and allow reader to inspect generated SQL structure.
  virtual void SelfTest(ITestComparer* cmp) {
    // Do nothing
  }
};
```