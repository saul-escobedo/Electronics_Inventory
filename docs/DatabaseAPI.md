# Database API

This document will explain the basics on how to use the database using the application's official database API.

## Introduction & Purpose

For this project, the Database interface is abstracted by using an abstract interface class (found in `Database.hpp`). The advantage of this abstraction is to have a clear separation of the application's layers between the business logic and database, and it provides a unifed way of using the database throughout the application.

Additionally, it future proofs the application, so this makes the program able to use other database implementations (like MYSql or Postgres). Typically, SQLite is used as the primary database backend for local persistent storage, but, as an example, a "mock" backend could also be implemented for testing purposes.

## API Structure

The two primary things to know about is:
* The data structures that models electical components.
* The Database interface class

The data structures are a set of classes that represent each type of electric component (e.g. diodes, resistors, transistors). They are all located under the `app/include/electrical` directory. Each component is derived from the base class `ElectricalComponent`, and it has properties that all components typically have like part number, manufacturer, and quantity (in stock).

The `Database.hpp` interface (located in the `app/include/database` directory) is an abstract interface class that has a set of functions that the business logic of the application can use to access the database. The business logic (backend) of the application must NOT access the database's implementation directly. In this interface, we can do typical CRUD operations, and it could return a list of results depending on certain parameters.


## Simple CRUD Examples

We must first have an instance a `Database`. The implementation depends on which one is chosen, but, in this example, we are using `SQLiteDatabase`.

```c++
using namespace ecim;

SQLiteDatabase sqlite("inventory.sqlite");

Database* db = &sqlite;
```

Note that `SQLiteDatabase` is derived from `Database`, and the typical business logic of the application is expected to use only functions listed in `Database`. Therefore, any of the following operations shown here will work with other implmentations too.

Creating new components involves creating a new instance of an `ElectricalComponent` with it's properties set. For example, let's use the `Resistor` component.

```c++
// The base config sets the typical properties all components have.
// It sets ratings, name, partnumber, manufacturer, description, and quantity.
ElectronicComponent::BaseConfig config{
    defaultRatings,
    "R1",
    "ResistorCo",
    "V-RES-1k",
    "1/4 W 1k Resistor",
    100
};

// 1k ohm resistor
Resistor r1(config, 1000);
```

Adding this new resistor to the database is a simple as this:

```c++
db->addComponent(r1);
```

As usual, be mindful of errors; sometimes this can throw an exception. Therefore, it is best to do these operations in a `try`/`catch` block.

Next, we can retieve a component if we know their ID:

```c++
// Keep in mind, this could throw an exception if the component with this id is not found
auto resistor = db->getComponent(1093);
```

Removing components is a similar process:

```c++
// The removed component is returned if needed, or null if not found
auto component = db->removeComponent(1093);
```

Updating the component's properties can usually follow this flow:

```c++
// Get the instance of the component already in the database
auto component = db->getComponent(64);

// We've had gain more stock if this component, let's increase its quantity
component->addQuantity(5)

// Let's save this back to the database
db->editComponent(component->ID(), *component);
```

The ID of the component must be explicitly set to avoid ambigutity and unexpected behavior (i.e. editing the wrong component when `component`'s ID was mistakely changed).

## Mass Quieries

If we need to get a list of components from the database, we'd use *Mass Quiries*. There are several parameters that could be set in order to refine the search and presentation of the items, and it will return a `MassQueryResult` containing statistics and the items themselves.

For instance, if we want to get a list of resistors, we'd do:

```c++
using namespace ecim;

// Set the parameters on how to perform a search
MassQueryConfig qcfg {
    // We only want to see 20 items at a time (first page)
    Pagination(1, 20),
    // Sort resistors from least to most resistive
    SortOrder::Acending,
    // We want it to be sorted by resistance
    Resistor::Property::Resistance,
    // We want resistors with a reistance of at least 500 ohms or greater
    { {Resistor::Property::Resistance, GreaterThan, 500} },
};

// Perform the serach and return the list of items encapsulated in MassQueryResult
MassQueryResult result = db->getAllComponentsByType(ElectronicComponent::Type::Resistor, qcfg);

// Get number of items found under criteria
std::cout << "Num items: " << result.totalItems << std::endl;

// Print part number for each item
for(int i = 0; i < result.totalItems; i++) {
    std::cout << i <<" : " << result.items[i]->partNumber();
}
```

## To be continued

This documentation of the API is not fully complete because the API is still developing, but it should give a good overview on how the application will interact with the database layer.
