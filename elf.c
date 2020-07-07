#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <elf.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>

void switch64(FILE* fp, Elf64_Word type);
void switch32(FILE* fp, Elf32_Word type);

int readfile(char *str){              //str为文件名
	struct stat st;	              //stat结构体，linux中描述文件属性的结构
	long sys_size;
	size_t rt;
	
	//打开文件
	FILE *fp=fopen(str,"r");
	if(!fp){
		perror("open error");
		exit(1);
	}

	//获得文件长度
	fseek(fp,0,SEEK_END);
	sys_size=ftell(fp);
	rewind(fp);
	
	//根据文件长度判断是不是elf文件
	if(sys_size<sizeof(Elf32_Ehdr)||sys_size<sizeof(Elf64_Ehdr))
	{
		printf("The file isn't a ELF file!\n");
		exit(1);
	}
	
	//根据Magic（魔数）来判断是不是elf文件
	char buf[EI_NIDENT]={0};
	rt=fread(buf,1,EI_NIDENT,fp);
	if(buf[0]!=0x7f && buf[1]!=0x45&&buf[2]!=0x4c&&buf[3]!=0x46){
		printf("The file isn't a ELF file!\n");
		exit(0);
	}
	rewind(fp);
	
	//魔数的第五个16进制数代表文件位数，返回对应的文件位数
	if(buf[4]==0x02)
		return 64;
	else
		return 32;
	
}

//打印elf文件头函数
int writeHead64(FILE* fp, Elf64_Ehdr* ehdr, Elf64_Phdr* phdr, Elf64_Shdr* shdr) {
	int i;
	fprintf(fp,"Magic : ");
		for (i = 0;i < 16; i++)
			fprintf(fp,"%x ", ehdr->e_ident[i]);
	fprintf(fp, "\n");
	fprintf(fp, "e_typ : %x\n", ehdr->e_type);
	fprintf(fp, "e_machine : %x\n", ehdr->e_machine);
	fprintf(fp, "e_version : %x\n", ehdr->e_version);
	fprintf(fp, "e_entry : %x\n", ehdr->e_entry);
	fprintf(fp, "e_phoff : %x\n", ehdr->e_phoff);
	fprintf(fp, "e_shoff : %x\n", ehdr->e_shoff);
	fprintf(fp, "e_flags : %x\n", ehdr->e_flags);
	fprintf(fp, "e_ehsize : %x\n", ehdr->e_ehsize);
	fprintf(fp, "e_phentsize : %x\n", ehdr->e_phentsize);
	fprintf(fp, "e_phnum : %x\n", ehdr->e_phnum);
	fprintf(fp, "e_shentsize : %x\n", ehdr->e_shentsize);
	fprintf(fp, "e_shnum : %x\n", ehdr->e_shnum);
	fprintf(fp, "e_shstrndx : %x\n", ehdr->e_shstrndx);
	fprintf(fp, "\n");
}

int writeHead32(FILE* fp, Elf32_Ehdr* ehdr, Elf32_Phdr* phdr, Elf32_Shdr* shdr) {
	int i;
	fprintf(fp,"Magic : ");
		for (i = 0;i < 16; i++)
			fprintf(fp,"%x ", ehdr->e_ident[i]);
	fprintf(fp, "\n");
	fprintf(fp, "e_typ : %x\n", ehdr->e_type);
	fprintf(fp, "e_machine : %x\n", ehdr->e_machine);
	fprintf(fp, "e_version : %x\n", ehdr->e_version);
	fprintf(fp, "e_entry : %x\n", ehdr->e_entry);
	fprintf(fp, "e_phoff : %x\n", ehdr->e_phoff);
	fprintf(fp, "e_shoff : %x\n", ehdr->e_shoff);
	fprintf(fp, "e_flags : %x\n", ehdr->e_flags);
	fprintf(fp, "e_ehsize : %x\n", ehdr->e_ehsize);
	fprintf(fp, "e_phentsize : %x\n", ehdr->e_phentsize);
	fprintf(fp, "e_phnum : %x\n", ehdr->e_phnum);
	fprintf(fp, "e_shentsize : %x\n", ehdr->e_shentsize);
	fprintf(fp, "e_shnum : %x\n", ehdr->e_shnum);
	fprintf(fp, "e_shstrndx : %x\n", ehdr->e_shstrndx);
	fprintf(fp, "\n");
}

//打印section部分和program部分的函数
int writeSection64(FILE* fp, uint8_t* buf,Elf64_Ehdr* ehdr, Elf64_Phdr* phdr, Elf64_Shdr* shdr) {
	
	char* StringTable, * interp;
	StringTable = &buf[shdr[ehdr->e_shstrndx].sh_offset];
	int i;
	fprintf(fp, "Section header list:\n");
	for (i = 1;i < ehdr->e_shnum;i++) {
		fprintf(fp, "[%2d]\t%s\t", i - 1, &StringTable[shdr[i].sh_name]);
		switch64(fp,shdr[i].sh_type);
		fprintf(fp, "\t0x%x\n", shdr[i].sh_addr);
	}

	fprintf(fp, "\nProgram header list:\n");
	for (i = 0;i < ehdr->e_phnum;i++) {
		fprintf(fp, "[%d]", i);
		switch (phdr[i].p_type) {
		case PT_LOAD:
			if (phdr[i].p_offset == 0)
				fprintf(fp, "\tPT_LOAD\tText segment\t0x%x\n", phdr[i].p_vaddr);
			else
				fprintf(fp, "\tPT_LOAD\tData segment\t0x%x\n", phdr[i].p_vaddr);
			break;
		case PT_INTERP:
			interp = strdup((char*)&buf[phdr[i].p_offset]);
			fprintf(fp, "\tPT_INTERP\tInterpreter\t%s\n", interp);
			break;
		case PT_NOTE:
			fprintf(fp, "\tPT_NOTE\tNote segment\t0x%x\n", phdr[i].p_vaddr);
			break;
		case PT_DYNAMIC:
			fprintf(fp, "\tPT_DYNAMIC\tDynamic segment\t0x%x\n", phdr[i].p_vaddr);
			break;
		case PT_PHDR:
			fprintf(fp, "\tPT_PHDR\tPhdr segment\t0x%x\n", phdr[i].p_vaddr);
			break;
		case PT_GNU_EH_FRAME:
			fprintf(fp, "\tPT_GNU_EH_FRAME\tsegment\t0x%x\n", phdr[i].p_vaddr);
			break;
		case PT_GNU_STACK:
			fprintf(fp, "\tPT_GNU_STACK\tsegment\t0x%x\n", phdr[i].p_vaddr);
			break;
		case PT_GNU_RELRO:
			fprintf(fp, "\tPT_GNU_RELRO\tsegment\t0x%x\n", phdr[i].p_vaddr);
			break;
		default:
			fprintf(fp, "some segment can not parse\n");
			break;
		}
	}
	return 0;
}

int writeSection32(FILE* fp, uint8_t *buf,Elf32_Ehdr* ehdr, Elf32_Phdr* phdr, Elf32_Shdr* shdr) {
	char* StringTable, * interp;
	int i;
	StringTable = &buf[shdr[ehdr->e_shstrndx].sh_offset];
	fprintf(fp, "Section header list:\n");
	for (i = 1;i < ehdr->e_shnum;i++) {
		fprintf(fp, "[%2d]\t%s\t", i - 1, &StringTable[shdr[i].sh_name]);
		switch32(fp,shdr[i].sh_type);
		fprintf(fp, "\t0x%x\n", shdr[i].sh_addr);
	}

	fprintf(fp, "\nProgram header list\n");
	for (i = 0;i < ehdr->e_phnum;i++) {
		fprintf(fp, "[%d]", i);
		switch (phdr[i].p_type) {
		case PT_LOAD:
			if (phdr[i].p_offset == 0)
				fprintf(fp, "\tPT_LOAD\tText segment\t0x%x\n", phdr[i].p_vaddr);
			else
				fprintf(fp, "\tPT_LOAD\tData segment\t0x%x\n", phdr[i].p_vaddr);
			break;
		case PT_INTERP:
			interp = strdup((char*)&buf[phdr[i].p_offset]);
			fprintf(fp, "\tPT_INTERP\tInterpreter\t%s\n", interp);
			break;
		case PT_NOTE:
			fprintf(fp, "\tPT_NOTE\tNote segment\t0x%x\n", phdr[i].p_vaddr);
			break;
		case PT_DYNAMIC:
			fprintf(fp, "\tPT_DYNAMIC\tDynamic segment\t0x%x\n", phdr[i].p_vaddr);
			break;
		case PT_PHDR:
			fprintf(fp, "\tPT_PHDR\tPhdr segment\t0x%x\n", phdr[i].p_vaddr);
			break;
		case PT_GNU_EH_FRAME:
			fprintf(fp, "\tPT_GNU_EH_FRAME\tsegment\t0x%x\n", phdr[i].p_vaddr);
			break;
		case PT_GNU_STACK:
			fprintf(fp, "\tPT_GNU_STACK\tsegment\t0x%x\n", phdr[i].p_vaddr);
			break;
		case PT_GNU_RELRO:
			fprintf(fp, "\tPT_GNU_RELRO\tsegment\t0x%x\n", phdr[i].p_vaddr);
			break;
		default:
			fprintf(fp, "some segment can not parse\n");
			break;
		}
	}
	return 0;
}

int main(int argc,char *argv[])
{
	if(argc < 2){
		printf("your input is illegal\n");
		exit(0);
	}
	int sys_type=readfile(argv[1]);   //获得文件位数
	//创建文本文件
	char *filename;
	char* lastname = ".txt";
	filename = (char *)malloc(strlen(argv[1]) + strlen(lastname));
	sprintf(filename, "%s%s", argv[1], lastname);
	FILE* nf = fopen(filename, "w");
	uint8_t* buf;
	//判断文件位数，选择对应方法
	if(sys_type==64){
		int fd=open(argv[1],O_RDONLY);
		struct stat st;
		fstat(fd,&st);
		//创建对应结构体指针
		Elf64_Ehdr *ehdr;
		Elf64_Phdr *phdr;
		Elf64_Shdr *shdr;
		//将读取的文件映射到缓存中
		buf = mmap(NULL,st.st_size,PROT_READ,MAP_PRIVATE,fd,0);
		if(buf == MAP_FAILED){
			perror("mmap error!");
			exit(1);
		}
		//将缓存中相应空间转换为对应结构体指针
		ehdr = (Elf64_Ehdr *)buf;
		phdr = (Elf64_Phdr *)&buf[ehdr->e_phoff];
		shdr = (Elf64_Shdr *)&buf[ehdr->e_shoff];
		//打印数据
		writeHead64(nf, ehdr, phdr, shdr);
		writeSection64(nf,buf, ehdr, phdr, shdr);
		printf("write success!\n");
	

	}
	else{
		int fd=open(argv[1],O_RDONLY);
		struct stat st;
		fstat(fd,&st);

		Elf32_Ehdr *ehdr;
		Elf32_Phdr *phdr;
		Elf32_Shdr *shdr;
	
		buf = mmap(NULL,st.st_size,PROT_READ,MAP_PRIVATE,fd,0);
		if(buf == MAP_FAILED){
			perror("mmap error!");
			exit(1);
		}

		ehdr = (Elf32_Ehdr *)buf;
		phdr = (Elf32_Phdr *)&buf[ehdr->e_phoff];
		shdr = (Elf32_Shdr *)&buf[ehdr->e_shoff];
		writeHead32(nf, ehdr, phdr, shdr);
		writeSection32(nf,buf, ehdr, phdr, shdr);
		printf("write success!\n");	
	}	
}

//将宏定义的类型转换为对应字符串
void switch64(FILE *fp,Elf64_Word type)
{
	switch(type){
		case SHT_NULL:
			fprintf(fp,"SHT_NULL");
			break;
		case SHT_PROGBITS:
			fprintf(fp,"SHT_PROGBITS");
			break;
		case SHT_SYMTAB:
			fprintf(fp,"SHT_SYMTAB");
			break;
		case SHT_STRTAB:
			fprintf(fp,"SHT_STRTAB");
			break;
		case SHT_RELA:
			fprintf(fp, "SHT_RELA");
			break;
		case SHT_HASH:
			fprintf(fp, "SHT_HASH");
			break;
		case SHT_DYNAMIC:
			fprintf(fp, "SHT_DYNAMIC");
			break;
		case SHT_NOTE:
			fprintf(fp, "SHT_NOTE");
			break;
		case SHT_NOBITS:
			fprintf(fp, "SHT_NOBITS");
			break;
		case SHT_REL:
			fprintf(fp, "SHT_REL");
			break;
		case SHT_SHLIB:
			fprintf(fp, "SHT_SHLIB");
			break;
		case SHT_DYNSYM:
			fprintf(fp, "SHT_DYNSYM");
			break;
		case SHT_INIT_ARRAY:
			fprintf(fp, "SHT_INIT_ARRAY");
			break;
		case SHT_FINI_ARRAY:
			fprintf(fp, "SHT_FINI_ARRAY");
			break;
		case SHT_PREINIT_ARRAY:
			fprintf(fp, "SHT_PREINIT_ARRAY");
			break;
		case SHT_GROUP:
			fprintf(fp, "SHT_GROUP");
			break;
		case SHT_SYMTAB_SHNDX:
			fprintf(fp, "SHT_SYMTAB_SHNDX");
			break;
		case SHT_LOOS:
			fprintf(fp, "SHT_LOOP");
			break;
		case SHT_SUNW_COMDAT:
			fprintf(fp, "SHT_SUNW_COMDAT");
			break;
		case SHT_SUNW_syminfo:
			fprintf(fp, "SHT_SUNW_syminfo");
			break;
		case SHT_LOPROC:
			fprintf(fp, "SHT_LOPROC");
			break;
		case SHT_HIPROC:
			fprintf(fp, "SHT_HITROC");
			break;
		case SHT_LOUSER:
			fprintf(fp, "SHT_LOUSER");
			break;
		case SHT_HIUSER:
			fprintf(fp, "SHT_HITUSER");
			break;
		default:
			fprintf(fp, "NOT_FIND");
			break;
	}

}

void switch32(FILE* fp, Elf32_Word type)
{
	switch (type) {
	case SHT_NULL:
		fprintf(fp, "SHT_NULL");
		break;
	case SHT_PROGBITS:
		fprintf(fp, "SHT_PROGBITS");
		break;
	case SHT_SYMTAB:
		fprintf(fp, "SHT_SYMTAB");
		break;
	case SHT_STRTAB:
		fprintf(fp, "SHT_STRTAB");
		break;
	case SHT_RELA:
		fprintf(fp, "SHT_RELA");
		break;
	case SHT_HASH:
		fprintf(fp, "SHT_HASH");
		break;
	case SHT_DYNAMIC:
		fprintf(fp, "SHT_DYNAMIC");
		break;
	case SHT_NOTE:
		fprintf(fp, "SHT_NOTE");
		break;
	case SHT_NOBITS:
		fprintf(fp, "SHT_NOBITS");
		break;
	case SHT_REL:
		fprintf(fp, "SHT_REL");
		break;
	case SHT_SHLIB:
		fprintf(fp, "SHT_SHLIB");
		break;
	case SHT_DYNSYM:
		fprintf(fp, "SHT_DYNSYM");
		break;
	case SHT_INIT_ARRAY:
		fprintf(fp, "SHT_INIT_ARRAY");
		break;
	case SHT_FINI_ARRAY:
		fprintf(fp, "SHT_FINI_ARRAY");
		break;
	case SHT_PREINIT_ARRAY:
		fprintf(fp, "SHT_PREINIT_ARRAY");
		break;
	case SHT_GROUP:
		fprintf(fp, "SHT_GROUP");
		break;
	case SHT_SYMTAB_SHNDX:
		fprintf(fp, "SHT_SYMTAB_SHNDX");
		break;
	case SHT_LOOS:
		fprintf(fp, "SHT_LOOP");
		break;
	case SHT_SUNW_COMDAT:
		fprintf(fp, "SHT_SUNW_COMDAT");
		break;
	case SHT_SUNW_syminfo:
		fprintf(fp, "SHT_SUNW_syminfo");
		break;
	case SHT_LOPROC:
		fprintf(fp, "SHT_LOPROC");
		break;
	case SHT_HIPROC:
		fprintf(fp, "SHT_HITROC");
		break;
	case SHT_LOUSER:
		fprintf(fp, "SHT_LOUSER");
		break;
	case SHT_HIUSER:
		fprintf(fp, "SHT_HITUSER");
		break;
	default:
		fprintf(fp, "NOT_FIND");
		break;
	}

}



