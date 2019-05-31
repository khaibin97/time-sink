/**
 * @author See Contributors.txt for code contributors and overview of BadgerDB.
 *
 * @section LICENSE
 * Copyright (c) 2012 Database Group, Computer Sciences Department, University of Wisconsin-Madison.
 */

#include "btree.h"
#include "filescan.h"
#include "exceptions/bad_index_info_exception.h"
#include "exceptions/bad_opcodes_exception.h"
#include "exceptions/bad_scanrange_exception.h"
#include "exceptions/no_such_key_found_exception.h"
#include "exceptions/scan_not_initialized_exception.h"
#include "exceptions/index_scan_completed_exception.h"
#include "exceptions/file_not_found_exception.h"
#include "exceptions/end_of_file_exception.h"
#include "exceptions/page_pinned_exception.h"
//#define DEBUG

namespace badgerdb
{

// -----------------------------------------------------------------------------
// BTreeIndex::BTreeIndex -- Constructor
// -----------------------------------------------------------------------------

BTreeIndex::BTreeIndex(const std::string & relationName,
		std::string & outIndexName,
		BufMgr *bufMgrIn,
		const int attrByteOffset,
		const Datatype attrType)
{
	// create index name
	//from program write up
	std:: ostringstream idxStr;
	idxStr << relationName << '.' << attrByteOffset;
	std::string indexName = idxStr.str();

	//initialize the vartiable we gonna use
	scanExecuting = false;
	leafOccupancy = INTARRAYLEAFSIZE;
	nodeOccupancy = INTARRAYNONLEAFSIZE;
	outIndexName = indexName;
	bufMgr = bufMgrIn;

	// check if this index file already exists or not.
	if (!(File::exists(indexName)))
	{
		//since no file, make root
		rootNode = true;
		this->attrByteOffset = attrByteOffset;
		this->attributeType = attrType;

		this -> file = new BlobFile(indexName, true);

		Page* pageMeta;
		bufMgr->allocPage(file, headerPageNum, pageMeta);

		Page* rootP;//rootPage
		bufMgr->allocPage(file, rootPageNum, rootP); 

		// update meata info
		IndexMetaInfo* metaInfo = (IndexMetaInfo*)pageMeta;
		strcpy(metaInfo->relationName, relationName.c_str());
		metaInfo->rootPageNo = rootPageNum;
		metaInfo->attrByteOffset = attrByteOffset;
		metaInfo->attrType = attributeType;

		LeafNodeInt* root = (LeafNodeInt *)rootP;
		root->rightSibPageNo = 0;      

		bufMgr->unPinPage(file, rootPageNum, true);
		bufMgr->unPinPage(file, headerPageNum, true);

		//from main.cpp
		// scan records and insert into the Btree
		FileScan fscan(relationName, bufMgr);
		try
		{
			RecordId recordid;
			while (1)
			{
				fscan.scanNext(recordid);
				std::string recordStr = fscan.getRecord();
				const char *record = recordStr.c_str();
				void* key = (void *)(record + attrByteOffset);
				insertEntry(key,recordid);
			}
		}
		catch(EndOfFileException e)
		{
			//do nothing	
		}
		bufMgr->flushFile(file);//flush
		
		
	} else {
		file = new BlobFile(indexName, false);//exception

		// read the meta page
		headerPageNum = file->getFirstPageNo();

		//get the root page num from the meta page
		Page* pageMeta;
		bufMgr->readPage(file, headerPageNum, pageMeta);
		IndexMetaInfo* metaInfo = (IndexMetaInfo*) pageMeta;
		
		//if matches
		if (metaInfo->attrByteOffset == attrByteOffset) {
			this->attrByteOffset = metaInfo->attrByteOffset;
			attributeType = metaInfo->attrType;
			rootPageNum = metaInfo->rootPageNo;
		} else {
			throw BadIndexInfoException("");
		}
     		
		//read the root page
		Page* rootP;
		bufMgr->readPage(file, rootPageNum, rootP);
		//check for root and set root since it may change
		if (rootPageNum == 2) {
			rootNode = true;
		}
		bufMgr->unPinPage(file, headerPageNum, false);
	}
}


// -----------------------------------------------------------------------------
// BTreeIndex::~BTreeIndex -- destructor
// -----------------------------------------------------------------------------

BTreeIndex::~BTreeIndex()
{
	bufMgr->flushFile(file);
	delete file;
	file = NULL;//defensive programming
	scanExecuting = false;
}


// -----------------------------------------------------------------------------
// BTreeIndex::lookup
// -----------------------------------------------------------------------------

const void BTreeIndex::lookup(PageId currPageNo, RIDKeyPair<int> entry, PageKeyPair<int>& newChildEntry)
{
	// Fetch current page
	Page *cPage;
	bufMgr->readPage(file, currPageNo, cPage);
	NonLeafNodeInt* cNode = (NonLeafNodeInt*) (cPage);


	// Find the index to traverse.
	int index = 0;
	for (index = 0; (index < nodeOccupancy) && (cNode->pageNoArray[index] != 0); index++) {
		if ((cNode->keyArray[index] >= entry.key))
			break;//found
	}

	//index is last page
	if (cNode->pageNoArray[index] == 0){
		if (index > 0){
	    	   index--;//to get correct page or not out of bounds
		}
	}

	// get the page number of the child
	PageId childPageNo = cNode->pageNoArray[index];
	Page* childPage;
	PageKeyPair<int> newPage;
	PageKeyPair<int> anotherNewPage;

	// insert leaf split if full
	if (cNode->level == 1) {

		bufMgr->readPage(file, childPageNo, childPage);

		LeafNodeInt* childNode = (LeafNodeInt*)(childPage);
		if ((childNode->ridArray[leafOccupancy-1]).page_number == 0) {
			insertAtLeaf(childNode, entry);
		} 
		else {
			// split 
			splitLeaf(entry, childNode, newPage);
			// insert depending on space
			if (cNode->pageNoArray[nodeOccupancy] == 0) { 
				insertAtNonLeaf(cNode, newPage);
			} else {
				splitNonLeaf(newPage, cNode, anotherNewPage);
				newChildEntry = anotherNewPage;	
			}
		}
		//unpin used page
		bufMgr->unPinPage(file, childPageNo, true); 
		bufMgr->unPinPage(file, currPageNo, true);
		return;
	}

	PageKeyPair<int> newPageKeyPair;
	newPageKeyPair.set(0,0);
	bufMgr->unPinPage(file,currPageNo,false); 
	lookup(childPageNo, entry, newPageKeyPair);

	Page* readCurrent;
	bufMgr->readPage(file,currPageNo,readCurrent);

	if (newPageKeyPair.pageNo != 0) {
		newPage.set(newPageKeyPair.pageNo, newPageKeyPair.key);
		if (cNode->pageNoArray[nodeOccupancy]==0) {
			insertAtNonLeaf(cNode, newPage);
  		}  else {
  			splitNonLeaf(newPage, cNode, anotherNewPage);
			newChildEntry = anotherNewPage;
  		}
  	}

  	bufMgr->unPinPage(file, currPageNo, (newPageKeyPair.pageNo !=0) ? true : false); 
}

//--------------------------------------------------------------------------------------------------------------
//BTreeIndex::findKeyInLeaf
//---------------------------------------------------------------------------------------------------------------
const void BTreeIndex::findKeyInLeaf(PageId leaf){
	Page* lPage;
	bufMgr->readPage(file, leaf, lPage);
	LeafNodeInt* leafNode = (LeafNodeInt*)(lPage);
    int i = 0;
    while(leafNode->ridArray[i].page_number != 0){

    	if(lowOp == GT && leafNode->keyArray[i] > lowValInt){
            // Start record found, set the currentPage number and the currentPage data 
            currentPageNum = leaf;
            currentPageData = lPage;
            nextEntry = i;
            return;
    	}else if(lowOp == GTE && leafNode->keyArray[i] >= lowValInt){
            // Start record found, set the current page number and the current page data
            currentPageNum = leaf;
            currentPageData = lPage;
            nextEntry = i;
            return;
        }
        i++;
    }
                
	currentPageNum = leaf; //To unpin page

    throw NoSuchKeyFoundException();// No node was found // throw exception
}

// ----------------------------------------------------------------------------------------------------------------
// BTreeIndex::findStartRecordID
// ----------------------------------------------------------------------------------------------------------------

const void BTreeIndex::findStartRecordID(PageId rootNo){
	// Check if root node is a leaf 
	if(rootNode) // the tree has only root node
    {
        findKeyInLeaf(rootNo);
		return;
    }

	Page *rootPage;
	//Read the page into the buffer pool
	bufMgr->readPage(file, rootNo, rootPage);
    NonLeafNodeInt *node = (NonLeafNodeInt*)(rootPage);
	// Find the page to the leaf node
    int i = 0;
    while(lowOp == GT && i < INTARRAYNONLEAFSIZE-1 && lowValInt > node->keyArray[i] && (node->pageNoArray[i+1] != 0)){
         i++;
    }
    while(lowOp == GTE && i < INTARRAYNONLEAFSIZE-1 && lowValInt >= node->keyArray[i] && (node->pageNoArray[i+1] != 0)){
          i++;
    }
    if(node->level ==  1) //  One above the Leaf node
    {
	bufMgr->unPinPage(file, rootNo, false);
	// Search for the key in leaf node 
	findKeyInLeaf(node->pageNoArray[i]); 
    }
    else{
        // Non - leaf node check if it satifies the conditions 
		// No record found here, unpin page and move on to the next page
		bufMgr->unPinPage(file, rootNo, false);
        findStartRecordID(node->pageNoArray[i]);
    }
}

// -----------------------------------------------------------------------------
// BTreeIndex::insertEntry
// -----------------------------------------------------------------------------

const void BTreeIndex::insertEntry(const void *key, const RecordId rid) 
{
	if(rootNode){
		RIDKeyPair<int> entry;
		entry.set(rid, *((int*)(key)));
		BTreeIndex::insertAtRoot(entry);
	} else{

		RIDKeyPair<int> entry;
		entry.set(rid, *((int*)(key)));
		
		PageKeyPair<int> newChildEntry;
		newChildEntry.set(0,entry.key);
		BTreeIndex::lookup(rootPageNum, entry, newChildEntry);
		//create a new root if needed
		PageId prevRoot = rootPageNum;
		Page* rootP;
		bufMgr->readPage(file, prevRoot, rootP);

		if (newChildEntry.pageNo != 0) {
			// allocate a new page
			Page* newRoot;
			PageId newPageNo;
			bufMgr->allocPage(file, newPageNo, newRoot);

			// set values in the new node
			NonLeafNodeInt* newNode = (NonLeafNodeInt*)newRoot;
			newNode->level = 0;
			newNode->pageNoArray[0] = prevRoot;
			newNode->pageNoArray[1] = newChildEntry.pageNo;
			newNode->keyArray[0] = newChildEntry.key;

			// update root page info and pageMeta
			rootNode = false;
			rootPageNum = newPageNo;
			Page* headerPage;
			bufMgr->readPage(file, headerPageNum, headerPage);
			IndexMetaInfo * pageMeta = (IndexMetaInfo*) headerPage;
			pageMeta->rootPageNo = rootPageNum;
				
			//unpin the rest
			bufMgr->unPinPage(file, newPageNo, true);
			bufMgr->unPinPage(file, headerPageNum, true);
			
		}
			//unpin prevroot because old root 
			bufMgr->unPinPage(file, prevRoot, true);
	}

}

// -----------------------------------------------------------------------------
// BTreeIndex::insertAtRoot
// -----------------------------------------------------------------------------

const void BTreeIndex::insertAtRoot(RIDKeyPair<int> entry)
{
        Page* cPage;
        bufMgr->readPage(file, rootPageNum, cPage);
        PageId prevRoot = rootPageNum;
        LeafNodeInt* leafNode = (LeafNodeInt*) (cPage);

        // leaf not full
        if(!(leafNode->ridArray[leafOccupancy-1].page_number != 0)){
                insertAtLeaf(leafNode, entry);
        } else {
                PageKeyPair<int> newPage;
                splitLeaf(entry, leafNode, newPage);
				//set new root node
				// allocate a new page
				Page* newRoot;
				PageId newPageNo;
				bufMgr->allocPage(file, newPageNo, newRoot);

				// set values in the new node
				NonLeafNodeInt* newNode = (NonLeafNodeInt*)newRoot;
				newNode->level = 1;
				newNode->pageNoArray[0] = rootPageNum;
				newNode->pageNoArray[1] = newPage.pageNo;
				newNode->keyArray[0] = newPage.key;

				// update root page info and pageMeta
				rootNode = false;
				rootPageNum = newPageNo;
				Page* headerPage;
				bufMgr->readPage(file, headerPageNum, headerPage);
				IndexMetaInfo * pageMeta = (IndexMetaInfo*) headerPage;
				pageMeta->rootPageNo = rootPageNum;
				
				//unpin the rest
				bufMgr->unPinPage(file, newPageNo, true);
				bufMgr->unPinPage(file, headerPageNum, true);
        }
		//unpin since not root anymore
        bufMgr->unPinPage(file, prevRoot, true);
}

// -----------------------------------------------------------------------------
// BTreeIndex::insertAtLeaf
// ----------------------------------------------------------------------------

void BTreeIndex::insertAtLeaf(LeafNodeInt* leafNode, RIDKeyPair<int> entry){

	//find the index for insert
	int index;
	for (index = 0; index < leafOccupancy && (leafNode->ridArray[index].page_number != 0); index++) {
	if (leafNode->keyArray[index] >= entry.key)
		break;	
	}

	//make space for insert
	for (int i = leafOccupancy - 1; i > index; i--){
		leafNode->ridArray[i] = leafNode->ridArray[i-1];
		leafNode->keyArray[i] = leafNode->keyArray[i-1];      
	}

	// insert the entry at right position
	leafNode->ridArray[index] = entry.rid;
	leafNode->keyArray[index] = entry.key;

}

// -----------------------------------------------------------------------------
// BTreeIndex::insertAtNonLeaf
// ----------------------------------------------------------------------------

void BTreeIndex::insertAtNonLeaf(NonLeafNodeInt* nonLeafNode, PageKeyPair<int> entry){

	//find index for insert
    int index ;
    for (index = 0; ((nonLeafNode->pageNoArray[index]) != 0 && (index < leafOccupancy)); index++) {
    	if (nonLeafNode->keyArray[index] >= entry.key)
		break;
	}

	//make space
	for (int i = nodeOccupancy - 1; i > index; i--){
		nonLeafNode->keyArray[i] = nonLeafNode->keyArray[i-1];
		nonLeafNode->pageNoArray[i+1] = nonLeafNode->pageNoArray[i];
	}

	// insert the entry where appropriate
	if (nonLeafNode->pageNoArray[index] == 0) {
        nonLeafNode->pageNoArray[index] = entry.pageNo;
		nonLeafNode->keyArray[index-1] = entry.key;
	} else {
		nonLeafNode->pageNoArray[index+1] = entry.pageNo;
		nonLeafNode->keyArray[index] = entry.key;
    	}
}


// -----------------------------------------------------------------------------
// BTreeIndex::splitLeaf
// ----------------------------------------------------------------------------

	const void BTreeIndex::splitLeaf(RIDKeyPair<int> entry, LeafNodeInt * lNode, PageKeyPair<int> & nPage) {

		//We allocate a new page 
		PageId PageNo;
		Page* Page;
		bufMgr->allocPage(file, PageNo, Page);

		// Cast created page into a Leaf node
		LeafNodeInt* nNode = (LeafNodeInt*) Page;

		// determining the posdle position 
		int pos = leafOccupancy / 2 + 1;

		//chaining leafs 
		nNode->rightSibPageNo = lNode->rightSibPageNo;
		lNode->rightSibPageNo = PageNo;

		// assign values 
		for (int i = pos; i < leafOccupancy; i++) {
			nNode->keyArray[i - pos] = lNode->keyArray[i];
			nNode->ridArray[i - pos] = lNode->ridArray[i];
			lNode->ridArray[i].page_number = 0;
		}

		//determining where to place entry 
		if (entry.key < nNode->keyArray[0]) {
			insertAtLeaf(lNode, entry);
		}
		else {
			insertAtLeaf(nNode, entry);
		}

		// set entry for return
		nPage.set(PageNo, nNode->keyArray[0]);

		bufMgr->unPinPage(file, PageNo, true);
	}

// -----------------------------------------------------------------------------
// BTreeIndex::splitNonLeaf
// ----------------------------------------------------------------------------

const void BTreeIndex::splitNonLeaf(PageKeyPair<int> entry, NonLeafNodeInt * nlNode, PageKeyPair<int> & newInsertedPage) {

	//allocate new page
	PageId newPageNo;
	Page* nPage;
	bufMgr->allocPage(file, newPageNo, nPage);

	// cast created page into non leaf node 
	NonLeafNodeInt* nNode = (NonLeafNodeInt*)(nPage);

	// find pos point
	int pos = nodeOccupancy / 2 + 1;

	// set the level
	nNode->level = nlNode->level;

	// assign values 
	for (int i = pos; i < nodeOccupancy; i++) {
		nNode->keyArray[i - pos] = nlNode->keyArray[i];
		nNode->pageNoArray[i - pos] = nlNode->pageNoArray[i];
	}

	nNode->pageNoArray[nodeOccupancy - pos] = nlNode->pageNoArray[nodeOccupancy];
	nlNode->pageNoArray[nodeOccupancy] = 0;

	// determining insert position 
	if (entry.key < nNode->keyArray[0]) {
		insertAtNonLeaf(nlNode, entry);
	}
	else {
		insertAtNonLeaf(nNode, entry);
	}

	// set the values for return
	newInsertedPage.set(newPageNo, nNode->keyArray[0]);
	bufMgr->unPinPage(file, newPageNo, true);
}

// -----------------------------------------------------------------------------
// BTreeIndex::startScan
// -----------------------------------------------------------------------------

const void BTreeIndex::startScan(const void* lowValParm,
				   const Operator lowOpParm,
				   const void* highValParm,
				   const Operator highOpParm)
{
	if(lowOpParm == LT || lowOpParm == LTE)
		throw BadOpcodesException();
	if(highOpParm == GT || highOpParm == GTE)
		throw BadOpcodesException();
	if(*(int*)highValParm< *(int*)lowValParm)
                throw BadScanrangeException();
	// Check for errors and initialize scan variables
        if(scanExecuting == true)
                endScan();
	scanExecuting = true;
	if(attributeType == 0){
                lowValInt = *(int*)lowValParm;
                highValInt = *(int*) highValParm;
        }
        lowOp = lowOpParm;
        highOp = highOpParm;
	// Traverse the BTree to find the record ID for the first record
        findStartRecordID(rootPageNum);
}

// -----------------------------------------------------------------------------
// BTreeIndex::scanNext
// -----------------------------------------------------------------------------

const void BTreeIndex::scanNext(RecordId& outRid) 
{
	//No scan has started
	if(scanExecuting == false)
                throw ScanNotInitializedException();

        LeafNodeInt *cPage = reinterpret_cast<LeafNodeInt*> (currentPageData);
	// Scan the next records based on the highOp condition

        if(highOp == LT  && cPage->keyArray[nextEntry] <  highValInt ){ // Satisfies the condition 
                outRid = cPage->ridArray[nextEntry];
                nextEntry++;
        }
        else if(highOp == LTE && cPage->keyArray[nextEntry] <= highValInt ){
                outRid = cPage->ridArray[nextEntry];
                nextEntry++;
        }
        else{
                // No More records found 
                throw IndexScanCompletedException();
        }
        if(cPage->ridArray[nextEntry].page_number == 0 || nextEntry >= INTARRAYLEAFSIZE) // Move to the next page if exists
        {
                // Unpin the older page
		// Get the page number of the next page
		PageId nextPage = cPage->rightSibPageNo;
		if(nextPage == 0)// End of the tree
			throw IndexScanCompletedException();
		bufMgr->unPinPage(file, currentPageNum, false);
		currentPageNum = nextPage;
		// Set the nextEntry to 0 th of the next Page
		nextEntry = 0;
		bufMgr->readPage(file, currentPageNum, currentPageData);
        }
}

// -----------------------------------------------------------------------------
// BTreeIndex::endScan
// -----------------------------------------------------------------------------
//
const void BTreeIndex::endScan() 
{
	// If no scan is initialized 
        if(!scanExecuting){
                throw ScanNotInitializedException();
        }
	bufMgr->unPinPage(file, currentPageNum, false);
        // A scan is currently executing 
        // Reset all the scan variables
	currentPageNum = 0;
	currentPageData = NULL;
        scanExecuting = false;
        nextEntry = -1;
        currentPageNum = 0;
	highValInt = 0;
	lowValInt = 0;
}
}