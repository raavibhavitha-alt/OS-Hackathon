#include <stdio.h>
#include <stdlib.h>

#define MAX_PAGES 64
#define MAX_TLB   16
#define MAX_LOG   200

int main() {
    int page_size, num_pages, tlb_size;
    int page_table[MAX_PAGES];
    int tlb_page[MAX_TLB], tlb_frame[MAX_TLB];
    int tlb_count = 0, next = 0;
    int hits = 0, misses = 0, faults = 0;

    // log of every access (for the web report)
    int n = 0, l_addr[MAX_LOG], l_page[MAX_LOG], l_phys[MAX_LOG], l_res[MAX_LOG];
    const char *names[] = {"TLB HIT", "TLB MISS", "PAGE FAULT", "INVALID"};

    printf("Page size (bytes): ");
    scanf("%d", &page_size);
    printf("Number of pages (max %d): ", MAX_PAGES);
    scanf("%d", &num_pages);
    printf("TLB size (max %d): ", MAX_TLB);
    scanf("%d", &tlb_size);

    if (page_size <= 0 || num_pages <= 0 || num_pages > MAX_PAGES ||
        tlb_size <= 0 || tlb_size > MAX_TLB) {
        printf("Invalid input\n");
        return 1;
    }

    printf("Enter frame for each page (-1 = not in memory)\n");
    for (int i = 0; i < num_pages; i++) {
        printf("Page %d -> frame: ", i);
        scanf("%d", &page_table[i]);
    }

    int addr;
    printf("\nEnter virtual address (-1 to quit): ");
    while (scanf("%d", &addr) == 1 && addr != -1 && n < MAX_LOG) {
        int page = addr / page_size, offset = addr % page_size;
        int res, phys = -1;

        if (addr < 0 || page >= num_pages) {
            res = 3;
        } else {
            int frame = -1;
            for (int i = 0; i < tlb_count; i++)
                if (tlb_page[i] == page) { frame = tlb_frame[i]; break; }

            if (frame != -1) {
                hits++; res = 0;
                phys = frame * page_size + offset;
            } else {
                misses++;
                frame = page_table[page];
                if (frame == -1) {
                    faults++; res = 2;
                } else {
                    res = 1;
                    phys = frame * page_size + offset;
                    int idx;
                    if (tlb_count < tlb_size) idx = tlb_count++;
                    else { idx = next; next = (next + 1) % tlb_size; }
                    tlb_page[idx] = page;
                    tlb_frame[idx] = frame;
                }
            }
        }
        l_addr[n] = addr; l_page[n] = page; l_phys[n] = phys; l_res[n] = res; n++;
        printf("%s\nEnter virtual address (-1 to quit): ", names[res]);
    }

    int total = hits + misses;
    FILE *f = fopen("report.html", "w");
    fprintf(f, "<html><head><title>TLB Simulator</title><style>"
        "body{font-family:Arial;margin:30px}table{border-collapse:collapse}"
        "td,th{border:1px solid #999;padding:6px 12px;text-align:center}"
        "th{background:#333;color:#fff}.h{background:#c8f7c5}.m{background:#fff3b0}"
        ".f{background:#f7c5c5}.i{background:#ddd}</style></head><body>");
    fprintf(f, "<h1>TLB and Page Table Simulator</h1>");
    fprintf(f, "<p>Page size: %d | Pages: %d | TLB size: %d (FIFO)</p>", page_size, num_pages, tlb_size);

    fprintf(f, "<h2>Page Table</h2><table><tr><th>Page</th><th>Frame</th></tr>");
    for (int i = 0; i < num_pages; i++) {
        fprintf(f, "<tr><td>%d</td>", i);
        if (page_table[i] == -1) fprintf(f, "<td>-</td></tr>");
        else fprintf(f, "<td>%d</td></tr>", page_table[i]);
    }
    fprintf(f, "</table>");

    fprintf(f, "<h2>Accesses</h2><table><tr><th>#</th><th>Virtual</th><th>Page</th><th>Result</th><th>Physical</th></tr>");
    const char *cls[] = {"h", "m", "f", "i"};
    for (int i = 0; i < n; i++) {
        fprintf(f, "<tr class='%s'><td>%d</td><td>%d</td>", cls[l_res[i]], i + 1, l_addr[i]);
        if (l_res[i] == 3) fprintf(f, "<td>-</td>"); else fprintf(f, "<td>%d</td>", l_page[i]);
        fprintf(f, "<td>%s</td>", names[l_res[i]]);
        if (l_phys[i] == -1) fprintf(f, "<td>-</td></tr>"); else fprintf(f, "<td>%d</td></tr>", l_phys[i]);
    }
    fprintf(f, "</table>");

    fprintf(f, "<h2>Final TLB</h2><table><tr><th>Page</th><th>Frame</th></tr>");
    for (int i = 0; i < tlb_count; i++)
        fprintf(f, "<tr><td>%d</td><td>%d</td></tr>", tlb_page[i], tlb_frame[i]);
    fprintf(f, "</table>");

    fprintf(f, "<h2>Stats</h2><p>Total: %d | Hits: %d | Misses: %d | Page faults: %d | Hit ratio: %.2f%%</p>",
            total, hits, misses, faults, total ? 100.0 * hits / total : 0.0);
    fprintf(f, "</body></html>");
    fclose(f);

    printf("\nOpening report.html in browser...\n");
#ifdef _WIN32
    system("start report.html");
#elif __APPLE__
    system("open report.html");
#else
    system("xdg-open report.html");
#endif
    return 0;
}
