
#include <QtTest>
#include <QtDebug>

#include <Control.h>
#include <Parser.h>
#include <AST.h>
#include <Semantic.h>
#include <Scope.h>
#include <Symbols.h>
#include <CoreTypes.h>
#include <Names.h>
#include <Literals.h>

CPLUSPLUS_USE_NAMESPACE

class tst_Semantic: public QObject
{
    Q_OBJECT

    Control control;

public:
    TranslationUnit *parse(const QByteArray &source,
                           TranslationUnit::ParseMode mode)
    {
        StringLiteral *fileId = control.findOrInsertFileName("<stdin>");
        TranslationUnit *unit = new TranslationUnit(&control, fileId);
        unit->setSource(source.constData(), source.length());
        unit->parse(mode);
        return unit;
    }

    class Document {
        Q_DISABLE_COPY(Document)

    public:
        Document(TranslationUnit *unit)
            : unit(unit), globals(new Scope())
        { }

        ~Document()
        { delete globals; }

        void check()
        {
            QVERIFY(unit);
            QVERIFY(unit->ast());
            Semantic sem(unit->control());
            TranslationUnitAST *ast = unit->ast()->asTranslationUnit();
            QVERIFY(ast);
            for (DeclarationAST *decl = ast->declarations; decl; decl = decl->next) {
                sem.check(decl, globals);
            }
        }

        TranslationUnit *unit;
        Scope *globals;
    };

    QSharedPointer<Document> document(const QByteArray &source)
    {
        TranslationUnit *unit = parse(source, TranslationUnit::ParseTranlationUnit);
        QSharedPointer<Document> doc(new Document(unit));
        doc->check();
        return doc;
    }

private slots:
    void function_declaration_1();
    void function_declaration_2();
    void function_definition_1();
    void nested_class_1();
};

void tst_Semantic::function_declaration_1()
{
    QSharedPointer<Document> doc = document("void foo();");
    QCOMPARE(doc->globals->symbolCount(), 1U);

    Declaration *decl = doc->globals->symbolAt(0)->asDeclaration();
    QVERIFY(decl);

    FullySpecifiedType declTy = decl->type();
    Function *funTy = declTy->asFunction();
    QVERIFY(funTy);
    QVERIFY(funTy->returnType()->isVoidType());
    QCOMPARE(funTy->argumentCount(), 0U);

    QVERIFY(decl->name()->isNameId());
    Identifier *funId = decl->name()->asNameId()->identifier();
    QVERIFY(funId);

    const QByteArray foo(funId->chars(), funId->size());
    QCOMPARE(foo, QByteArray("foo"));
}

void tst_Semantic::function_declaration_2()
{
    QSharedPointer<Document> doc = document("void foo(const QString &s);");
    QCOMPARE(doc->globals->symbolCount(), 1U);

    Declaration *decl = doc->globals->symbolAt(0)->asDeclaration();
    QVERIFY(decl);

    FullySpecifiedType declTy = decl->type();
    Function *funTy = declTy->asFunction();
    QVERIFY(funTy);
    QVERIFY(funTy->returnType()->isVoidType());
    QCOMPARE(funTy->argumentCount(), 1U);

    // check the formal argument.
    Argument *arg = funTy->argumentAt(0)->asArgument();
    QVERIFY(arg);
    QVERIFY(arg->name());
    QVERIFY(! arg->hasInitializer());

    // check the argument's name.
    NameId *argNameId = arg->name()->asNameId();
    QVERIFY(argNameId);

    Identifier *argId = argNameId->identifier();
    QVERIFY(argId);

    QCOMPARE(QByteArray(argId->chars(), argId->size()), QByteArray("s"));

    // check the type of the formal argument
    FullySpecifiedType argTy = arg->type();
    QVERIFY(argTy->isReferenceType());
    QVERIFY(argTy->asReferenceType()->elementType().isConst());
    NamedType *namedTy = argTy->asReferenceType()->elementType()->asNamedType();
    QVERIFY(namedTy);
    QVERIFY(namedTy->name());
    Identifier *namedTypeId = namedTy->name()->asNameId()->identifier();
    QVERIFY(namedTypeId);
    QCOMPARE(QByteArray(namedTypeId->chars(), namedTypeId->size()),
             QByteArray("QString"));

    QVERIFY(decl->name()->isNameId());
    Identifier *funId = decl->name()->asNameId()->identifier();
    QVERIFY(funId);

    const QByteArray foo(funId->chars(), funId->size());
    QCOMPARE(foo, QByteArray("foo"));
}

void tst_Semantic::function_definition_1()
{
    QSharedPointer<Document> doc = document("void foo() {}");
    QCOMPARE(doc->globals->symbolCount(), 1U);

    Function *funTy = doc->globals->symbolAt(0)->asFunction();
    QVERIFY(funTy);
    QVERIFY(funTy->returnType()->isVoidType());
    QCOMPARE(funTy->argumentCount(), 0U);

    QVERIFY(funTy->name()->isNameId());
    Identifier *funId = funTy->name()->asNameId()->identifier();
    QVERIFY(funId);

    const QByteArray foo(funId->chars(), funId->size());
    QCOMPARE(foo, QByteArray("foo"));
}

void tst_Semantic::nested_class_1()
{
    QSharedPointer<Document> doc = document(
"class Object {\n"
"    class Data;\n"
"    Data *d;\n"
"};\n"
"class Object::Data {\n"
"   Object *q;\n"
"};\n"
    );
    QCOMPARE(doc->globals->symbolCount(), 2U);

    Class *classObject = doc->globals->symbolAt(0)->asClass();
    QVERIFY(classObject);
    QVERIFY(classObject->name());
    NameId *classObjectNameId = classObject->name()->asNameId();
    QVERIFY(classObjectNameId);
    Identifier *objectId = classObjectNameId->identifier();
    QCOMPARE(QByteArray(objectId->chars(), objectId->size()), QByteArray("Object"));
    QCOMPARE(classObject->baseClassCount(), 0U);
    QEXPECT_FAIL("", "Requires support for forward classes", Continue);
    QCOMPARE(classObject->members()->symbolCount(), 2U);

    Class *classObjectData = doc->globals->symbolAt(1)->asClass();
    QVERIFY(classObjectData);
    QVERIFY(classObjectData->name());
    QualifiedNameId *q = classObjectData->name()->asQualifiedNameId();
    QVERIFY(q);
    QCOMPARE(q->nameCount(), 2U);
    QVERIFY(q->nameAt(0)->asNameId());
    QVERIFY(q->nameAt(1)->asNameId());
    QCOMPARE(q->nameAt(0), classObject->name());
    QCOMPARE(doc->globals->lookat(q->nameAt(0)->asNameId()->identifier()), classObject);

    Declaration *decl = classObjectData->memberAt(0)->asDeclaration();
    QVERIFY(decl);
    PointerType *ptrTy = decl->type()->asPointerType();
    QVERIFY(ptrTy);
    NamedType *namedTy = ptrTy->elementType()->asNamedType();
    QVERIFY(namedTy);
    QVERIFY(namedTy->name()->asNameId());
    QCOMPARE(namedTy->name()->asNameId()->identifier(), objectId);
}


QTEST_APPLESS_MAIN(tst_Semantic)
#include "tst_semantic.moc"
