/*=============================================================================
	UnitTest.cpp: Unit test implementation
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "UnitTest.h"

namespace Lightmass
{

/** Defines how the light is stored in the scene's light octree. */
struct FTestOctreeSemantics
{
	enum { MaxElementsPerLeaf = 16 };
	enum { MinInclusiveElementsPerNode = 7 };
	enum { MaxNodeDepth = 12 };
	enum { LoosenessDenominator = 16 };

	typedef TInlineAllocator<MaxElementsPerLeaf> ElementAllocator;

	FORCEINLINE static FBoxCenterAndExtent GetBoundingBox(const FLOAT& Element)
	{
		return FBoxCenterAndExtent(FVector4(0), FVector4(Element));
	}

	FORCEINLINE static UBOOL AreElementsEqual(const FLOAT& A,const FLOAT& B)
	{
		return A == B;
	}
};


class FTestCollisionDataProvider
{
	TkDOPTree<FTestCollisionDataProvider, WORD>& kDOP;
	FVector4 Vertex;
public:

	FTestCollisionDataProvider(TkDOPTree<FTestCollisionDataProvider, WORD>& InkDOP)
		: kDOP(InkDOP)
		, Vertex(0)
	{
	}

	/**
	 * Given an index, returns the position of the vertex
	 *
	 * @param Index the index into the vertices array
	 */
	FORCEINLINE const FVector4& GetVertex(WORD Index) const
	{
		return Vertex;
	}

	/** Returns additional information. */
	FORCEINLINE INT GetItemIndex(WORD MaterialIndex) const
	{
		return 0;
	}

	/**
	 * Returns the kDOPTree for this mesh
	 */
	FORCEINLINE const TkDOPTree<FTestCollisionDataProvider,WORD>& GetkDOPTree(void) const
	{
		return kDOP;
	}
	/**
	 * Returns the local to world for the component
	 */
	FORCEINLINE const FMatrix& GetLocalToWorld(void) const
	{
		return FMatrix::Identity;
	}

	/**
	 * Returns the world to local for the component
	 */
	FORCEINLINE const FMatrix GetWorldToLocal(void) const
	{
		return FMatrix::Identity;
	}

	/**
	 * Returns the local to world transpose adjoint for the component
	 */
	FORCEINLINE FMatrix GetLocalToWorldTransposeAdjoint(void) const
	{
		return FMatrix::Identity;
	}

	/**
	 * Returns the determinant for the component
	 */
	FORCEINLINE FLOAT GetDeterminant(void) const
	{
		return 0.0f;
	}
};

class FTestRunnable : public FRunnable
{
	UBOOL bStop;
public:
	virtual UBOOL Init(void) 
	{
		bStop = FALSE;
		return TRUE;
	}

	virtual DWORD Run(void)
	{
		for (INT i = 0; i < 10 && !bStop; i++)
		{
			debugf(TEXT("Thread counter %d"), i);
			appSleep(1.0f);
		}
		debugf(TEXT("Thread done!!"));

		return 0;
	}

	virtual void Stop(void)
	{
		bStop = TRUE;
	}

	virtual void Exit(void)
	{
	}

};

void TestLightmass()
{
	debugf(TEXT("\n\n"));
	debugf(TEXT("==============================================================================================="));
	debugf(TEXT("Running \"unit test\". This will take several seconds, and will end with an assertion."));
	debugf(TEXT("This is on purpose, as it's testing the callstack gathering..."));
	debugf(TEXT("==============================================================================================="));
	debugf(TEXT("\n\n"));

	void* Buf = appMalloc(1024);

	TArray<INT> TestArray;

	TestArray.AddItem(5);

	TArray<INT> ArrayCopy = TestArray;
	
	FVector4 TestVectorA(1, 0, 0, 1);
	FVector4 TestVectorB(1, 1, 1, 1);
	FVector4 TestVector = TestVectorA + TestVectorB;

	FString TestString = FString::Printf(TEXT("Copy has %d, Vector is [%.2f, %.2f, %.2f, %.2f]\n"), ArrayCopy(0), TestVector.X, TestVector.Y, TestVector.Z, TestVector.W);

	printf(*TestString);

	appFree(Buf);

	struct FAlignTester
	{
		BYTE A;
		FMatrix M1;
		BYTE B;
		FMatrix M2;
		BYTE C;
		FVector4 V;
	};
	FAlignTester AlignTest;
	checkf(((PTRINT)(&FMatrix::Identity) & 15) == 0, TEXT("Identity matrix unaligned"));
	checkf(((PTRINT)(&AlignTest.M1) & 15) == 0, TEXT("First matrix unaligned"));
	checkf(((PTRINT)(&AlignTest.M2) & 15) == 0, TEXT("Second matrix unaligned"));
	checkf(((PTRINT)(&AlignTest.V) & 15) == 0, TEXT("Vector unaligned"));

	FGuid Guid(1, 2, 3, 4);
	debugf(TEXT("Guid is %s"), *Guid.String());

	TMap<FString, INT> TestMap;
	TestMap.Set(FString(TEXT("Five")), 5);
	TestMap.Set(TEXT("Ten"), 10);

	debugf(TEXT("Map[Five] = %d, Map[Ten] = %d"), TestMap.FindRef(TEXT("Five")), TestMap.FindRef(FString(TEXT("Ten"))));

	FMatrix TestMatrix(FVector4(0, 0, 0.1f, 0), FVector4(0, 1.0f, 0, 0), FVector4(0.9f, 0, 0, 0), FVector4(0, 0, 0, 1.0f));

	debugf(TEXT("Mat=\n  [%0.2f, %0.2f, %0.2f, %0.2f]\n  [%0.2f, %0.2f, %0.2f, %0.2f]\n  [%0.2f, %0.2f, %0.2f, %0.2f]\n  [%0.2f, %0.2f, %0.2f, %0.2f]"), 
		TestMatrix.M[0][0], TestMatrix.M[0][1], TestMatrix.M[0][2], TestMatrix.M[0][3], 
		TestMatrix.M[1][0], TestMatrix.M[1][1], TestMatrix.M[1][2], TestMatrix.M[1][3], 
		TestMatrix.M[2][0], TestMatrix.M[2][1], TestMatrix.M[2][2], TestMatrix.M[2][3], 
		TestMatrix.M[3][0], TestMatrix.M[3][1], TestMatrix.M[3][2], TestMatrix.M[3][3]
		);

	TestMatrix = TestMatrix.Transpose();

	debugf(TEXT("Transposed Mat=\n  [%0.2f, %0.2f, %0.2f, %0.2f]\n  [%0.2f, %0.2f, %0.2f, %0.2f]\n  [%0.2f, %0.2f, %0.2f, %0.2f]\n  [%0.2f, %0.2f, %0.2f, %0.2f]"), 
		TestMatrix.M[0][0], TestMatrix.M[0][1], TestMatrix.M[0][2], TestMatrix.M[0][3], 
		TestMatrix.M[1][0], TestMatrix.M[1][1], TestMatrix.M[1][2], TestMatrix.M[1][3], 
		TestMatrix.M[2][0], TestMatrix.M[2][1], TestMatrix.M[2][2], TestMatrix.M[2][3], 
		TestMatrix.M[3][0], TestMatrix.M[3][1], TestMatrix.M[3][2], TestMatrix.M[3][3]
		);

	TestMatrix = TestMatrix.Transpose().Inverse();

	debugf(TEXT("Inverted Mat=\n  [%0.2f, %0.2f, %0.2f, %0.2f]\n  [%0.2f, %0.2f, %0.2f, %0.2f]\n  [%0.2f, %0.2f, %0.2f, %0.2f]\n  [%0.2f, %0.2f, %0.2f, %0.2f]"), 
		TestMatrix.M[0][0], TestMatrix.M[0][1], TestMatrix.M[0][2], TestMatrix.M[0][3], 
		TestMatrix.M[1][0], TestMatrix.M[1][1], TestMatrix.M[1][2], TestMatrix.M[1][3], 
		TestMatrix.M[2][0], TestMatrix.M[2][1], TestMatrix.M[2][2], TestMatrix.M[2][3], 
		TestMatrix.M[3][0], TestMatrix.M[3][1], TestMatrix.M[3][2], TestMatrix.M[3][3]
		);

	debugf(TEXT("sizeof FDirectionalLight = %d, FLight = %d, FDirectionalLightData = %d"), sizeof(FDirectionalLight), sizeof(FLight), sizeof(FDirectionalLightData));

	TOctree<FLOAT, FTestOctreeSemantics> TestOctree(FVector4(0), 10.0f);
	TestOctree.AddElement(5);


	// kDOP test
	TkDOPTree<FTestCollisionDataProvider, WORD> TestkDOP;
	FTestCollisionDataProvider TestDataProvider(TestkDOP);
	FCheckResult TestResult;
	
	FkDOPBuildCollisionTriangle<WORD> TestTri(0, FVector4(0,0,0,0), FVector4(1,1,1,0), FVector4(2,2,2,0), INDEX_NONE, INDEX_NONE, FALSE, TRUE);
	TArray<FkDOPBuildCollisionTriangle<WORD> > TestTriangles;
	TestTriangles.AddItem(TestTri);

	TestkDOP.Build(TestTriangles);
	
	debugf(TEXT("\nStarting a thread"));
	FTestRunnable* TestRunnable = new FTestRunnable;
	// create a thread with the test runnable, and let it auto-delete itself
	FRunnableThread* TestThread = GThreadFactory->CreateThread(TestRunnable, "TestRunnable");


	DOUBLE Start = appSeconds();
	debugf(TEXT("\nWaiting 4 seconds"), Start);
	appSleep(4.0f);
	debugf(TEXT("%.2f seconds have passed, killing thread"), appSeconds() - Start);
	
	// wait for thread to end
	DOUBLE KillStart = appSeconds();
	TestRunnable->Stop();
	TestThread->WaitForCompletion();
	
	delete TestRunnable;
	GThreadFactory->Destroy(TestThread);
	
	debugf(TEXT("It took %.2f seconds to kill the thread [should be < 1 second]"), appSeconds() - KillStart);


	debugf(TEXT("\n\n"));

	checkf(5 == 2, TEXT("And boom goes the dynamite\n"));
}

} // namespace
