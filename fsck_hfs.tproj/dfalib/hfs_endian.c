/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * hfs_endian.c
 *
 * This file implements endian swapping routines for the HFS/HFS Plus
 * volume format.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <architecture/byte_order.h>
#include <hfs/hfs_format.h>

#include "hfs_endian.h"

#undef ENDIAN_DEBUG

static void hfs_swap_HFSPlusForkData (HFSPlusForkData *src);
static int hfs_swap_HFSPlusBTInternalNode (BlockDescriptor *src, HFSCatalogNodeID fileID, int unswap);
static int hfs_swap_HFSBTInternalNode (BlockDescriptor *src, HFSCatalogNodeID fileID, int unswap);

/*
 * hfs_swap_HFSMasterDirectoryBlock
 *
 *  Specially modified to swap parts of the finder info
 */
void
hfs_swap_HFSMasterDirectoryBlock (
    void *buf
)
{
    HFSMasterDirectoryBlock *src = (HFSMasterDirectoryBlock *)buf;

    src->drSigWord		= SWAP_BE16 (src->drSigWord);
    src->drCrDate		= SWAP_BE32 (src->drCrDate);
    src->drLsMod		= SWAP_BE32 (src->drLsMod);
    src->drAtrb			= SWAP_BE16 (src->drAtrb);
    src->drNmFls		= SWAP_BE16 (src->drNmFls);
    src->drVBMSt		= SWAP_BE16 (src->drVBMSt);
    src->drAllocPtr		= SWAP_BE16 (src->drAllocPtr);
    src->drNmAlBlks		= SWAP_BE16 (src->drNmAlBlks);
    src->drAlBlkSiz		= SWAP_BE32 (src->drAlBlkSiz);
    src->drClpSiz		= SWAP_BE32 (src->drClpSiz);
    src->drAlBlSt		= SWAP_BE16 (src->drAlBlSt);
    src->drNxtCNID		= SWAP_BE32 (src->drNxtCNID);
    src->drFreeBks		= SWAP_BE16 (src->drFreeBks);

    /* Don't swap drVN */

    src->drVolBkUp		= SWAP_BE32 (src->drVolBkUp);
    src->drVSeqNum		= SWAP_BE16 (src->drVSeqNum);
    src->drWrCnt		= SWAP_BE32 (src->drWrCnt);
    src->drXTClpSiz		= SWAP_BE32 (src->drXTClpSiz);
    src->drCTClpSiz		= SWAP_BE32 (src->drCTClpSiz);
    src->drNmRtDirs		= SWAP_BE16 (src->drNmRtDirs);
    src->drFilCnt		= SWAP_BE32 (src->drFilCnt);
    src->drDirCnt		= SWAP_BE32 (src->drDirCnt);

    /* Swap just the 'blessed folder' in drFndrInfo */
    src->drFndrInfo[0]	= SWAP_BE32 (src->drFndrInfo[0]);

    src->drEmbedSigWord	= SWAP_BE16 (src->drEmbedSigWord);
	src->drEmbedExtent.startBlock = SWAP_BE16 (src->drEmbedExtent.startBlock);
	src->drEmbedExtent.blockCount = SWAP_BE16 (src->drEmbedExtent.blockCount);

    src->drXTFlSize		= SWAP_BE32 (src->drXTFlSize);
	src->drXTExtRec[0].startBlock = SWAP_BE16 (src->drXTExtRec[0].startBlock);
	src->drXTExtRec[0].blockCount = SWAP_BE16 (src->drXTExtRec[0].blockCount);
	src->drXTExtRec[1].startBlock = SWAP_BE16 (src->drXTExtRec[1].startBlock);
	src->drXTExtRec[1].blockCount = SWAP_BE16 (src->drXTExtRec[1].blockCount);
	src->drXTExtRec[2].startBlock = SWAP_BE16 (src->drXTExtRec[2].startBlock);
	src->drXTExtRec[2].blockCount = SWAP_BE16 (src->drXTExtRec[2].blockCount);

    src->drCTFlSize		= SWAP_BE32 (src->drCTFlSize);
	src->drCTExtRec[0].startBlock = SWAP_BE16 (src->drCTExtRec[0].startBlock);
	src->drCTExtRec[0].blockCount = SWAP_BE16 (src->drCTExtRec[0].blockCount);
	src->drCTExtRec[1].startBlock = SWAP_BE16 (src->drCTExtRec[1].startBlock);
	src->drCTExtRec[1].blockCount = SWAP_BE16 (src->drCTExtRec[1].blockCount);
	src->drCTExtRec[2].startBlock = SWAP_BE16 (src->drCTExtRec[2].startBlock);
	src->drCTExtRec[2].blockCount = SWAP_BE16 (src->drCTExtRec[2].blockCount);
}

/*
 * hfs_swap_HFSPlusVolumeHeader
 */
void
hfs_swap_HFSPlusVolumeHeader (
    void *buf
)
{
    HFSPlusVolumeHeader *src = (HFSPlusVolumeHeader *)buf;

    src->signature			= SWAP_BE16 (src->signature);
    src->version			= SWAP_BE16 (src->version);
    src->attributes			= SWAP_BE32 (src->attributes);
    src->lastMountedVersion	= SWAP_BE32 (src->lastMountedVersion);

    /* Don't swap reserved */

    src->createDate			= SWAP_BE32 (src->createDate);
    src->modifyDate			= SWAP_BE32 (src->modifyDate);
    src->backupDate			= SWAP_BE32 (src->backupDate);
    src->checkedDate		= SWAP_BE32 (src->checkedDate);
    src->fileCount			= SWAP_BE32 (src->fileCount);
    src->folderCount		= SWAP_BE32 (src->folderCount);
    src->blockSize			= SWAP_BE32 (src->blockSize);
    src->totalBlocks		= SWAP_BE32 (src->totalBlocks);
    src->freeBlocks			= SWAP_BE32 (src->freeBlocks);
    src->nextAllocation		= SWAP_BE32 (src->nextAllocation);
    src->rsrcClumpSize		= SWAP_BE32 (src->rsrcClumpSize);
    src->dataClumpSize		= SWAP_BE32 (src->dataClumpSize);
    src->nextCatalogID		= SWAP_BE32 (src->nextCatalogID);
    src->writeCount			= SWAP_BE32 (src->writeCount);
    src->encodingsBitmap	= SWAP_BE64 (src->encodingsBitmap);

    /* Don't swap finderInfo */

    hfs_swap_HFSPlusForkData (&src->allocationFile);
    hfs_swap_HFSPlusForkData (&src->extentsFile);
    hfs_swap_HFSPlusForkData (&src->catalogFile);
    hfs_swap_HFSPlusForkData (&src->attributesFile);
    hfs_swap_HFSPlusForkData (&src->startupFile);
}

/*
 * hfs_swap_HFSPlusForkData
 *
 *  There's still a few spots where we still need to swap the fork data.
 */
void
hfs_swap_HFSPlusForkData (
    HFSPlusForkData *src
)
{
    unsigned int i;

	src->logicalSize		= SWAP_BE64 (src->logicalSize);

	src->clumpSize			= SWAP_BE32 (src->clumpSize);
	src->totalBlocks		= SWAP_BE32 (src->totalBlocks);

    for (i = 0; i < kHFSPlusExtentDensity; i++) {
        src->extents[i].startBlock	= SWAP_BE32 (src->extents[i].startBlock);
        src->extents[i].blockCount	= SWAP_BE32 (src->extents[i].blockCount);
    }
}


/*
 * hfs_swap_BTNode
 *
 *  NOTE: This operation is not naturally symmetric.
 *        We have to determine which way we're swapping things.
 */
int
hfs_swap_BTNode (
    BlockDescriptor *src,
    int isHFSPlus,
    HFSCatalogNodeID fileID,
    int unswap
)
{
    BTNodeDescriptor *srcDesc = src->buffer;
    UInt16 *srcOffs = NULL;

    UInt32 i;
    int error = 0;


#ifdef ENDIAN_DEBUG
    if (unswap == 0) {
        printf ("BE -> LE Swap\n");
    } else if (unswap == 1) {
        printf ("LE -> BE Swap\n");
    } else if (unswap == 3) {
        printf ("Not swapping descriptors\n");
    } else {
        printf ("%s This is impossible", "hfs_swap_BTNode:");
        exit(99);
    }
#endif

    /* If we are doing a swap */
    if (unswap == 0) {
		if (*((UInt16 *)((char *)src->buffer + (src->blockSize - sizeof (UInt16)))) == 0x000e) {
			printf("hfs_swap_BTNode: node was left in little endian!\n");
			exit(99);
			return (0);
		}
	
        srcDesc->fLink		= SWAP_BE32 (srcDesc->fLink);
        srcDesc->bLink		= SWAP_BE32 (srcDesc->bLink);
    
        /* Don't swap srcDesc->kind */
        /* Don't swap srcDesc->height */
        /* Don't swap srcDesc->reserved */
    
        srcDesc->numRecords	= SWAP_BE16 (srcDesc->numRecords);
		        
        /* Swap the node offsets (including the free space one!) */
        srcOffs = (UInt16 *)((char *)src->buffer + (src->blockSize - ((srcDesc->numRecords + 1) * sizeof (UInt16))));

        /* Sanity check */
        if ((char *)srcOffs > ((char *)src->buffer + src->blockSize)) {
            printf ("%s Too many records in the B-Tree node", "hfs_swap_BTNode:");
            exit(99);
        }

        for (i = 0; i < srcDesc->numRecords + 1; i++) {
            srcOffs[i]	= SWAP_BE16 (srcOffs[i]);

            /* Sanity check */
            if (srcOffs[i] >= src->blockSize) {
                printf ("%s B-Tree node offset[%d] %d out of range (%d)", "hfs_swap_BTNode:", i, srcOffs[i], src->blockSize);
                exit(99);
            }
        }
    }
    
    /* Swap the records (ordered by frequency of access) */
    /* Swap a B-Tree internal node */
    if ((srcDesc->kind == kBTIndexNode) ||
        (srcDesc-> kind == kBTLeafNode)) {

        if (isHFSPlus) {
            error = hfs_swap_HFSPlusBTInternalNode (src, fileID, unswap);
        } else {
            error = hfs_swap_HFSBTInternalNode (src, fileID, unswap);
        }
        
    /* Swap a B-Tree map node */
    } else if (srcDesc-> kind == kBTMapNode) {
        /* Don't swap the bitmaps, they'll be done in the bitmap routines */
    
    /* Swap a B-Tree header node */
    } else if (srcDesc-> kind == kBTHeaderNode) {
        /* The header's offset is hard-wired because we cannot trust the offset pointers */
        BTHeaderRec *srcHead = (BTHeaderRec *)((char *)src->buffer + 14);
        
        srcHead->treeDepth		=	SWAP_BE16 (srcHead->treeDepth);
        
        srcHead->rootNode		=	SWAP_BE32 (srcHead->rootNode);
        srcHead->leafRecords	=	SWAP_BE32 (srcHead->leafRecords);
        srcHead->firstLeafNode	=	SWAP_BE32 (srcHead->firstLeafNode);
        srcHead->lastLeafNode	=	SWAP_BE32 (srcHead->lastLeafNode);
        
        srcHead->nodeSize		=	SWAP_BE16 (srcHead->nodeSize);
        srcHead->maxKeyLength	=	SWAP_BE16 (srcHead->maxKeyLength);
        
        srcHead->totalNodes		=	SWAP_BE32 (srcHead->totalNodes);
        srcHead->freeNodes		=	SWAP_BE32 (srcHead->freeNodes);
        
        srcHead->clumpSize		=	SWAP_BE32 (srcHead->clumpSize);
        srcHead->attributes		=	SWAP_BE32 (srcHead->attributes);

        /* Don't swap srcHead->reserved1 */
        /* Don't swap srcHead->btreeType */
        /* Don't swap srcHead->reserved2 */
        /* Don't swap srcHead->reserved3 */
        /* Don't swap bitmap */
    }
    
    /* If we are doing an unswap */
    if (unswap == 1) {
	if (*((UInt16 *)((char *)src->buffer + (src->blockSize - sizeof (UInt16)))) != 0x000e) {
		printf("hfs_swap_BTNode: node was not in little endian! (0x%04x)\n",
			*((UInt16 *)((char *)src->buffer + (src->blockSize - sizeof (UInt16)))));
		return (0);
	}

        /* Swap the node descriptor */
        srcDesc->fLink		= SWAP_BE32 (srcDesc->fLink);
        srcDesc->bLink		= SWAP_BE32 (srcDesc->bLink);
    
        /* Don't swap srcDesc->kind */
        /* Don't swap srcDesc->height */
        /* Don't swap srcDesc->reserved */
    
        /* Swap the node offsets (including the free space one!) */
        srcOffs = (UInt16 *)((char *)src->buffer + (src->blockSize - ((srcDesc->numRecords + 1) * sizeof (UInt16))));

        /* Sanity check */
        if ((char *)srcOffs > ((char *)src->buffer + src->blockSize)) {
            printf ("%s Too many records in the B-Tree node", "hfs_swap_BTNode:");
            exit(99);
        }

        for (i = 0; i < srcDesc->numRecords + 1; i++) {
            /* Sanity check */
            if (srcOffs[i] >= src->blockSize) {
                printf ("%s B-Tree node offset out of range", "hfs_swap_BTNode:");
                exit(99);
            }

            srcOffs[i]	= SWAP_BE16 (srcOffs[i]);
        }
        
        srcDesc->numRecords	= SWAP_BE16 (srcDesc->numRecords);
    }
    
    return (error);
}

static int
hfs_swap_HFSPlusBTInternalNode (
    BlockDescriptor *src,
    HFSCatalogNodeID fileID,
    int unswap
)
{
    BTNodeDescriptor *srcDesc = src->buffer;
    UInt16 *srcOffs = (UInt16 *)((char *)src->buffer + (src->blockSize - (srcDesc->numRecords * sizeof (UInt16))));

    UInt32 i;
    UInt32 j;

    if (fileID == kHFSExtentsFileID) {
        HFSPlusExtentKey *srcKey;
        HFSPlusExtentDescriptor *srcRec;
        
        for (i = 0; i < srcDesc->numRecords; i++) {
            srcKey = (HFSPlusExtentKey *)((char *)src->buffer + srcOffs[i]);

            if (!unswap) srcKey->keyLength		= SWAP_BE16 (srcKey->keyLength);
            srcRec = (HFSPlusExtentDescriptor *)((char *)srcKey + srcKey->keyLength + 2);
            if (unswap) srcKey->keyLength		= SWAP_BE16 (srcKey->keyLength);

            /* Don't swap srcKey->forkType */
            /* Don't swap srcKey->pad */

            srcKey->fileID			= SWAP_BE32 (srcKey->fileID);
            srcKey->startBlock		= SWAP_BE32 (srcKey->startBlock);
            
            /* Stop if this is just an index node */
            if (srcDesc->kind == kBTIndexNode) {
                *((UInt32 *)srcRec) = SWAP_BE32 (*((UInt32 *)srcRec));
                continue;
            }

            /* Swap the extent data */
            
            /* Swap each extent */
            for (j = 0; j < kHFSPlusExtentDensity; j++) {
                srcRec[j].startBlock	= SWAP_BE32 (srcRec[j].startBlock);
                srcRec[j].blockCount	= SWAP_BE32 (srcRec[j].blockCount);
            }
        }

    } else if (fileID == kHFSCatalogFileID || fileID == kHFSRepairCatalogFileID) {
        HFSPlusCatalogKey *srcKey;
        SInt16 *srcPtr;
        
        for (i = 0; i < srcDesc->numRecords; i++) {
            srcKey = (HFSPlusCatalogKey *)((char *)src->buffer + srcOffs[i]);

            if (!unswap) srcKey->keyLength			= SWAP_BE16 (srcKey->keyLength);
            srcPtr = (SInt16 *)((char *)srcKey + srcKey->keyLength + 2);
            if (unswap) srcKey->keyLength			= SWAP_BE16 (srcKey->keyLength);
            
            srcKey->parentID						= SWAP_BE32 (srcKey->parentID);

            if (!unswap) srcKey->nodeName.length	= SWAP_BE16 (srcKey->nodeName.length);
            for (j = 0; j < srcKey->nodeName.length; j++) {
                srcKey->nodeName.unicode[j]	= SWAP_BE16 (srcKey->nodeName.unicode[j]);
            }
            if (unswap) srcKey->nodeName.length	= SWAP_BE16 (srcKey->nodeName.length);
 
            /* Stop if this is just an index node */
            if (srcDesc->kind == kBTIndexNode) {
                *((UInt32 *)srcPtr) = SWAP_BE32 (*((UInt32 *)srcPtr));
                continue;
            }
           
            /* Swap the recordType field, if unswapping, leave to later */
            if (!unswap) srcPtr[0] = SWAP_BE16 (srcPtr[0]);
            
            if (srcPtr[0] == kHFSPlusFolderRecord) {
                HFSPlusCatalogFolder *srcRec = (HFSPlusCatalogFolder *)srcPtr;
                
                srcRec->flags				= SWAP_BE16 (srcRec->flags);
                srcRec->valence				= SWAP_BE32 (srcRec->valence);
                srcRec->folderID			= SWAP_BE32 (srcRec->folderID);
                srcRec->createDate			= SWAP_BE32 (srcRec->createDate);
                srcRec->contentModDate		= SWAP_BE32 (srcRec->contentModDate);
                srcRec->attributeModDate	= SWAP_BE32 (srcRec->attributeModDate);
                srcRec->accessDate			= SWAP_BE32 (srcRec->accessDate);
                srcRec->backupDate			= SWAP_BE32 (srcRec->backupDate);
                
                srcRec->bsdInfo.ownerID		= SWAP_BE32 (srcRec->bsdInfo.ownerID);
                srcRec->bsdInfo.groupID		= SWAP_BE32 (srcRec->bsdInfo.groupID);
    
                /* Don't swap srcRec->bsdInfo.adminFlags */
                /* Don't swap srcRec->bsdInfo.ownerFlags */
    
                srcRec->bsdInfo.fileMode			= SWAP_BE16 (srcRec->bsdInfo.fileMode);
                srcRec->bsdInfo.special.iNodeNum	= SWAP_BE32 (srcRec->bsdInfo.special.iNodeNum);
    
                srcRec->textEncoding		= SWAP_BE32 (srcRec->textEncoding);
    
                /* Don't swap srcRec->userInfo */
                /* Don't swap srcRec->finderInfo */
                /* Don't swap srcRec->reserved */
    
            } else if (srcPtr[0] == kHFSPlusFileRecord) {
                HFSPlusCatalogFile *srcRec = (HFSPlusCatalogFile *)srcPtr;
                
                srcRec->flags				= SWAP_BE16 (srcRec->flags);
    
                srcRec->fileID				= SWAP_BE32 (srcRec->fileID);
    
                srcRec->createDate			= SWAP_BE32 (srcRec->createDate);
                srcRec->contentModDate		= SWAP_BE32 (srcRec->contentModDate);
                srcRec->attributeModDate	= SWAP_BE32 (srcRec->attributeModDate);
                srcRec->accessDate			= SWAP_BE32 (srcRec->accessDate);
                srcRec->backupDate			= SWAP_BE32 (srcRec->backupDate);
    
                srcRec->bsdInfo.ownerID		= SWAP_BE32 (srcRec->bsdInfo.ownerID);
                srcRec->bsdInfo.groupID		= SWAP_BE32 (srcRec->bsdInfo.groupID);
    
                /* Don't swap srcRec->bsdInfo.adminFlags */
                /* Don't swap srcRec->bsdInfo.ownerFlags */
    
                srcRec->bsdInfo.fileMode			= SWAP_BE16 (srcRec->bsdInfo.fileMode);
                srcRec->bsdInfo.special.iNodeNum	= SWAP_BE32 (srcRec->bsdInfo.special.iNodeNum);
    
                srcRec->textEncoding		= SWAP_BE32 (srcRec->textEncoding);
    
                /* Don't swap srcRec->reserved1 */
                /* Don't swap srcRec->userInfo */
                /* Don't swap srcRec->finderInfo */
                /* Don't swap srcRec->reserved2 */
    
                hfs_swap_HFSPlusForkData (&srcRec->dataFork);
                hfs_swap_HFSPlusForkData (&srcRec->resourceFork);
            
            } else if ((srcPtr[0] == kHFSPlusFolderThreadRecord) ||
                       (srcPtr[0] == kHFSPlusFileThreadRecord)) {
    
                HFSPlusCatalogThread *srcRec = (HFSPlusCatalogThread *)srcPtr;
    
                /* Don't swap srcRec->reserved */
                
                srcRec->parentID						= SWAP_BE32 (srcRec->parentID);
                
                if (!unswap) srcRec->nodeName.length	= SWAP_BE16 (srcRec->nodeName.length);
                for (j = 0; j < srcRec->nodeName.length; j++) {
                    srcRec->nodeName.unicode[j]	= SWAP_BE16 (srcRec->nodeName.unicode[j]);
                }
                if (unswap) srcRec->nodeName.length		= SWAP_BE16 (srcRec->nodeName.length);

            } else {
                printf ("hfs_swap_BTNode: unrecognized catalog record type\n");
                exit(99);
            }
    
            /* If unswapping, we can safely unswap type field now */
            if (unswap) srcPtr[0] = SWAP_BE16 (srcPtr[0]);
        }
    } else if (fileID == kHFSAttributesFileID) {
    	HFSPlusAttrKey *srcKey;
    	HFSPlusAttrRecord *srcRec;
    	
    	for (i = 0; i < srcDesc->numRecords; i++) {
    		srcKey = (HFSPlusAttrKey *)((char *)src->buffer + srcOffs[i]);
    		
    		if (!unswap) srcKey->keyLength = SWAP_BE16(srcKey->keyLength);
    		srcRec = (HFSPlusAttrRecord *)((char *)srcKey + srcKey->keyLength + 2);
    		if (unswap) srcKey->keyLength = SWAP_BE16(srcKey->keyLength);
    		
    		srcKey->fileID = SWAP_BE32(srcKey->fileID);
    		srcKey->startBlock = SWAP_BE32(srcKey->startBlock);
    		
    		if (!unswap) srcKey->attrNameLen = SWAP_BE16(srcKey->attrNameLen);
    		for (j = 0; j < srcKey->attrNameLen; j++)
    			srcKey->attrName[j] = SWAP_BE16(srcKey->attrName[j]);
    		if (unswap) srcKey->attrNameLen = SWAP_BE16(srcKey->attrNameLen);
    		
    		/* If this is an index node, just swap the child node number */
            if (srcDesc->kind == kBTIndexNode) {
                *((UInt32 *)srcRec) = SWAP_BE32 (*((UInt32 *)srcRec));
                continue;
            }
            
            /* Swap the data record */
            if (!unswap) srcRec->recordType = SWAP_BE32(srcRec->recordType);
            switch (srcRec->recordType) {
            	case kHFSPlusAttrInlineData:
            		/* We're not swapping the reserved fields */
            		srcRec->attrData.attrSize = SWAP_BE32(srcRec->attrData.attrSize);
            		/* Not swapping the attrData */
            		break;
            	case kHFSPlusAttrForkData:
            		/* We're not swapping the reserved field */
            		hfs_swap_HFSPlusForkData(&srcRec->forkData.theFork);
            		break;
            	case kHFSPlusAttrExtents:
            		/* We're not swapping the reserved field */
            		for (j = 0; j<kHFSPlusExtentDensity; j++) {
            			srcRec->overflowExtents.extents[j].startBlock =
            				SWAP_BE32(srcRec->overflowExtents.extents[j].startBlock);
            			srcRec->overflowExtents.extents[j].blockCount =
            				SWAP_BE32(srcRec->overflowExtents.extents[j].blockCount);
            		}
            		break;
            	default:
					printf ("hfs_swap_BTNode: unrecognized attribute record type\n");
					exit(99);
            }
            if (unswap) srcRec->recordType = SWAP_BE32(srcRec->recordType);
    	}
    } else {
        printf ("hfs_swap_BTNode: unrecognized B-Tree type\n");
        exit(99);
    }

    return (0);
}

static int
hfs_swap_HFSBTInternalNode (
    BlockDescriptor *src,
    HFSCatalogNodeID fileID,
    int unswap
)
{
    BTNodeDescriptor *srcDesc = src->buffer;
    UInt16 *srcOffs = (UInt16 *)((char *)src->buffer + (src->blockSize - (srcDesc->numRecords * sizeof (UInt16))));

    UInt32 i;
    UInt32 j;

    if (fileID == kHFSExtentsFileID) {
        HFSExtentKey *srcKey;
        HFSExtentDescriptor *srcRec;
        
        for (i = 0; i < srcDesc->numRecords; i++) {
            srcKey = (HFSExtentKey *)((char *)src->buffer + srcOffs[i]);

            /* Don't swap srcKey->keyLength */
            /* Don't swap srcKey->forkType */

            srcKey->fileID			= SWAP_BE32 (srcKey->fileID);
            srcKey->startBlock		= SWAP_BE16 (srcKey->startBlock);

            /* Point to record data (round up to even byte boundary) */
            srcRec = (HFSExtentDescriptor *)((char *)srcKey + ((srcKey->keyLength + 2) & ~1));
    
            /* Stop if this is just an index node */
            if (srcDesc->kind == kBTIndexNode) {
                *((UInt32 *)srcRec) = SWAP_BE32 (*((UInt32 *)srcRec));
                continue;
            }
            
            /* Swap each extent */
            for (j = 0; j < kHFSExtentDensity; j++) {
                srcRec[j].startBlock	= SWAP_BE16 (srcRec[j].startBlock);
                srcRec[j].blockCount	= SWAP_BE16 (srcRec[j].blockCount);
            }
        }
        
    } else if (fileID == kHFSCatalogFileID || fileID == kHFSRepairCatalogFileID) {
        HFSCatalogKey *srcKey;
        SInt16 *srcPtr;
        
        for (i = 0; i < srcDesc->numRecords; i++) {
            srcKey = (HFSCatalogKey *)((char *)src->buffer + srcOffs[i]);

            /* Don't swap srcKey->keyLength */
            /* Don't swap srcKey->reserved */

            srcKey->parentID			= SWAP_BE32 (srcKey->parentID);

            /* Don't swap srcKey->nodeName */

            /* Point to record data (round up to even byte boundary) */
            srcPtr = (SInt16 *)((char *)srcKey + ((srcKey->keyLength + 2) & ~1));
            
            /* Stop if this is just an index node */
            if (srcDesc->kind == kBTIndexNode) {
                *((UInt32 *)srcPtr) = SWAP_BE32 (*((UInt32 *)srcPtr));
                continue;
            }
    
            /* Swap the recordType field, if unswapping, leave to later */
            if (!unswap) srcPtr[0] = SWAP_BE16 (srcPtr[0]);
            
            if (srcPtr[0] == kHFSFolderRecord) {
                HFSCatalogFolder *srcRec = (HFSCatalogFolder *)srcPtr;
                
                srcRec->flags				= SWAP_BE16 (srcRec->flags);
                srcRec->valence				= SWAP_BE16 (srcRec->valence);
                
                srcRec->folderID			= SWAP_BE32 (srcRec->folderID);
                srcRec->createDate			= SWAP_BE32 (srcRec->createDate);
                srcRec->modifyDate			= SWAP_BE32 (srcRec->modifyDate);
                srcRec->backupDate			= SWAP_BE32 (srcRec->backupDate);
    
                /* Don't swap srcRec->userInfo */
                /* Don't swap srcRec->finderInfo */
                /* Don't swap resserved array */
    
            } else if (srcPtr[0] == kHFSFileRecord) {
                HFSCatalogFile *srcRec = (HFSCatalogFile *)srcPtr;
                
                srcRec->flags				= srcRec->flags;
                srcRec->fileType			= srcRec->fileType;
    
                /* Don't swap srcRec->userInfo */
    
                srcRec->fileID				= SWAP_BE32 (srcRec->fileID);
                
                srcRec->dataStartBlock		= SWAP_BE16 (srcRec->dataStartBlock);
                srcRec->dataLogicalSize		= SWAP_BE32 (srcRec->dataLogicalSize);
                srcRec->dataPhysicalSize	= SWAP_BE32 (srcRec->dataPhysicalSize);
                
                srcRec->rsrcStartBlock		= SWAP_BE16 (srcRec->rsrcStartBlock);
                srcRec->rsrcLogicalSize		= SWAP_BE32 (srcRec->rsrcLogicalSize);
                srcRec->rsrcPhysicalSize	= SWAP_BE32 (srcRec->rsrcPhysicalSize);
                
                srcRec->createDate			= SWAP_BE32 (srcRec->createDate);
                srcRec->modifyDate			= SWAP_BE32 (srcRec->modifyDate);
                srcRec->backupDate			= SWAP_BE32 (srcRec->backupDate);
    
                /* Don't swap srcRec->finderInfo */
    
                srcRec->clumpSize			= SWAP_BE16 (srcRec->clumpSize);
                
                /* Swap the two sets of extents as an array of six (three each) UInt16 */
                for (j = 0; j < kHFSExtentDensity * 2; j++) {
                    srcRec->dataExtents[j].startBlock	= SWAP_BE16 (srcRec->dataExtents[j].startBlock);
                    srcRec->dataExtents[j].blockCount	= SWAP_BE16 (srcRec->dataExtents[j].blockCount);
                }
    
                /* Don't swap srcRec->reserved */
                
            } else if ((srcPtr[0] == kHFSFolderThreadRecord) ||
                    (srcPtr[0] == kHFSFileThreadRecord)) {
    
                HFSCatalogThread *srcRec = (HFSCatalogThread *)srcPtr;
    
                /* Don't swap srcRec->reserved array */
    
                srcRec->parentID			= SWAP_BE32 (srcRec->parentID);
    
                /* Don't swap srcRec->nodeName */
    
            } else {
                printf ("%s unrecognized catalog record type", "hfs_swap_BTNode:");
                exit(99);
            }
    
            /* If unswapping, we can safely swap type now */
            if (unswap) srcPtr[0] = SWAP_BE16 (srcPtr[0]);
        }
        
    } else {
        printf ("%s unrecognized B-Tree type", "hfs_swap_BTNode:");
        exit(99);
    }

    return (0);
}