/*
 * ObjectType.hh
 *
 *  Created on: Jan 30, 2009
 *      Author: javier
 */

#ifndef OBJECTTYPE_HH_
#define OBJECTTYPE_HH_

#include <map>
#include <vector>

#include "PDBInterpreter.hh"
#include "TokenType.hh"
#include <boost/smart_ptr/shared_ptr.hpp>

namespace EUROPA {

class ObjectType;
typedef Id<ObjectType> ObjectTypeId;

class ObjectTypeMgr;
typedef Id<ObjectTypeMgr> ObjectTypeMgrId;

class ObjectFactory;
typedef Id<ObjectFactory> ObjectFactoryId;

/** Version of ObjectType for communication with other languages */
class PSObjectType {
public:
	  virtual ~PSObjectType() {}
	  virtual const std::string& getNameString() const = 0;
	  virtual const std::string& getParentName() const = 0;
	  virtual PSList<std::string> getMemberNames() const = 0;
	  virtual PSDataType* getMemberTypeRef(const std::string& name) const = 0;
          virtual PSList<PSTokenType*> getPredicates() const = 0;
          virtual PSList<PSTokenType*>  getPSTokenTypesByAttr( int attrMask ) const = 0;
};

class ObjectType: public PSObjectType
{
public:
    ObjectType(const std::string& name, const ObjectTypeId parent, bool isNative=false);
    virtual ~ObjectType();

    const ObjectTypeId getId() const;

    const DataTypeId getVarType() const; // Data type for a variable that holds a reference to an object

  virtual const std::string& getName() const;
    virtual const ObjectTypeId getParent() const;
    virtual bool isNative() const;

    virtual void addMember(const DataTypeId type, const std::string& name); // TODO: use DataType instead
    virtual const std::map<std::string,DataTypeId>& getMembers() const;
    virtual const DataTypeId getMemberType(const std::string& name) const;

    virtual void addObjectFactory(const ObjectFactoryId factory);
  virtual const std::map<std::string,ObjectFactoryId>& getObjectFactories() const;

    virtual void addTokenType(const TokenTypeId factory);
  virtual const std::map<std::string,TokenTypeId>& getTokenTypes() const;
    virtual const TokenTypeId getTokenType(const std::string& signature) const;
    virtual const TokenTypeId getParentType(const TokenTypeId factory) const;

    virtual std::string toString() const;

    void purgeAll(); // TODO: make protected after Schema API is fixed

    // From PSObjectType
	virtual const std::string& getNameString() const;
	virtual const std::string& getParentName() const;
	virtual PSList<std::string> getMemberNames() const;
	virtual PSDataType* getMemberTypeRef(const std::string& name) const;
	virtual PSList<PSTokenType*> getPredicates() const;
        virtual PSList<PSTokenType*>  getPSTokenTypesByAttr( int attrMask ) const;


protected:
    ObjectTypeId m_id;
    DataTypeId m_varType;
  std::string m_name;
    ObjectTypeId m_parent;
    bool m_isNative;
  std::map<std::string,ObjectFactoryId> m_objectFactories;
  std::map<std::string,TokenTypeId> m_tokenTypes;
  std::map<std::string,DataTypeId> m_members;
};


/**
 * @brief Manages metadata on ObjectTypes
 *
 */
class ObjectTypeMgr {
public:

	ObjectTypeMgr();
	virtual ~ObjectTypeMgr();

	const ObjectTypeMgrId getId() const;

    void registerObjectType(const ObjectTypeId objType);
    const ObjectTypeId getObjectType(const std::string& objType) const;
    std::vector<ObjectTypeId> getAllObjectTypes() const;

	/**
	 * @brief Helper method to compose full factory signature from type and arguments
	 * @param objectType The type of the object defined in a model.
	 * @param arguments The sequence of name/value pairs to be passed as arguments for construction of the object
	 * @return A ':' deliimited string of <objectType>:<arg0.type>:..:<argn.type>
	 */
	static std::string makeFactoryName(const std::string& objectType, const std::vector<const Domain*>& arguments);

	/**
	 * @brief Obtain the factory based on the type of object to create and the types of the arguments to the constructor
	 */
	ObjectFactoryId getFactory(const SchemaId schema, const std::string& objectType, const std::vector<const Domain*>& arguments, const bool doCheckError = true);

	/**
	 * @brief Add a factory to provide instantiation of particular concrete types based on a label.
	 */
	void registerFactory(const ObjectFactoryId factory);

	/**
	 * @brief Delete all meta-data stored.
	 */
	void purgeAll();

protected:
	ObjectTypeMgrId m_id;
  std::map<std::string, ObjectTypeId> m_objTypes;
  std::map<std::string, ObjectFactoryId> m_factories; // TODO: should delegate to object types instead
};

/**
 * @brief Each concrete class must provide an implementation for this.
 */
class ObjectFactory{
public:
  ObjectFactory(const std::string& signature);

  virtual ~ObjectFactory();

  const ObjectFactoryId getId() const;

  /**
   * @brief Return the type for which this factory is registered.
   */
  const std::string& getSignature() const;

  /**
   * @brief Retreive the type signature as a vector of type names.
   * TODO: re-write this so it actually returns DataTypes.
   */
  const std::vector<std::string>& getSignatureTypes() const;

  /**
   * @brief Create a root object instance
   * @see DbClient::createObject(const std::string& type, const std::string& name)
   * for the interpreted version createInstance = makeObject + evalConstructorBody
   */
  virtual ObjectId createInstance(const PlanDatabaseId planDb,
                  const std::string& objectType,
                  const std::string& objectName,
                  const std::vector<const Domain*>& arguments) const = 0;


 // TODO: when code generation goes away, InterpretedObjectFactory will be the base implementation
 // makeNewObject will still be the function to be overriden by native classes
 // evalConstructorBody will become protected and will probably never be overriden.
 // dummy implementation provided for now to be compatible with code generation.
 /**
   * @brief makes an instance of a new object, this is purely construction, initialization happens in evalConstructorBody
   */
 virtual ObjectId makeNewObject(
                          const PlanDatabaseId planDb,
                          const std::string& objectType,
                          const std::string& objectName,
                          const std::vector<const Domain*>& arguments) const;
  /**
   * @brief The body of the constructor after the object is created
   * any operations done by createInstance to the object after it is created must be done by this method
   * so that calls to "super()" in subclasses can be supported correctly
   */
  virtual void evalConstructorBody(ObjectId instance,
                                   const std::vector<const Domain*>& arguments) const;

private:
  ObjectFactoryId m_id;
  std::string m_signature;
  std::vector<std::string> m_signatureTypes;
};

// Call to super inside a constructor
class ExprConstructorSuperCall : public Expr
{
  public:
      ExprConstructorSuperCall(const std::string& superClassName,
                               const std::vector<Expr*>& argExprs);
      virtual ~ExprConstructorSuperCall();

      virtual DataRef eval(EvalContext& context) const;

      const std::string& getSuperClassName() const { return m_superClassName; }

      void evalArgs(EvalContext& context, std::vector<const Domain*>& arguments) const;

  protected:
      std::string m_superClassName;
      std::vector<Expr*> m_argExprs;
};

class InterpretedObjectFactory : public ObjectFactory
{
  public:
      InterpretedObjectFactory(
          const ObjectTypeId objType,
          const std::string& signature,
          const std::vector<std::string>& constructorArgNames,
          const std::vector<std::string>& constructorArgTypes,
          ExprConstructorSuperCall* superCallExpr,
          const std::vector<Expr*>& constructorBody,
          bool canMakeNewObject = false
      );

      virtual ~InterpretedObjectFactory();

  protected:
      // createInstance = makeNewObject + evalConstructorBody
      virtual ObjectId createInstance(
                              const PlanDatabaseId planDb,
                              const std::string& objectType,
                              const std::string& objectName,
                              const std::vector<const Domain*>& arguments) const;

      // Any exported C++ classes must register a factory for each C++ constructor
      // and override this method to call the C++ constructor
      virtual ObjectId makeNewObject(
                          const PlanDatabaseId planDb,
                          const std::string& objectType,
                          const std::string& objectName,
                          const std::vector<const Domain*>& arguments) const;

      virtual void evalConstructorBody(
                         ObjectId instance,
                         const std::vector<const Domain*>& arguments) const;

      bool checkArgs(const std::vector<const Domain*>& arguments) const;

      std::string                  m_className;
      std::vector<std::string>  m_constructorArgNames;
      std::vector<std::string>  m_constructorArgTypes;
      ExprConstructorSuperCall* m_superCallExpr;
      std::vector<Expr*>        m_constructorBody;
      bool                      m_canMakeNewObject;
      mutable EvalContext*      m_evalContext;
private:
  InterpretedObjectFactory(const InterpretedObjectFactory&);
  InterpretedObjectFactory& operator=(const InterpretedObjectFactory&);
};

class NativeObjectFactory : public InterpretedObjectFactory
{
  public:
      NativeObjectFactory(const ObjectTypeId objType, const std::string& signature)
          : InterpretedObjectFactory(
                objType,                    // objType
                signature,                  // signature
                std::vector<std::string>(), // ConstructorArgNames
                std::vector<std::string>(), // constructorArgTypes
                NULL,                       // SuperCallExpr
                std::vector<Expr*>(),       // constructorBody
                true                        // canCreateObjects
            )
      {
      }

      virtual ~NativeObjectFactory() {}

  protected:
      virtual ObjectId makeNewObject(
                          const PlanDatabaseId planDb,
                          const std::string& objectType,
                          const std::string& objectName,
                          const std::vector<const Domain*>& arguments) const = 0;
};

}


#endif /* OBJECTTYPE_HH_ */
