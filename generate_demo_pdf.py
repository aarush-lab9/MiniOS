#!/usr/bin/env python3
"""
generate_demo_pdf.py
Generates a comprehensive, publication-quality presentation and viva guide PDF:
'MiniOS_Website_Demo_Guide.pdf'
Saved in /Users/aarushdubey/Downloads/
"""

from reportlab.lib.pagesizes import letter
from reportlab.lib import colors
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether, HRFlowable
)
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.pdfgen import canvas

class NumberedCanvas(canvas.Canvas):
    """
    Two-pass canvas to dynamically compute and render total page count
    along with professional running headers and footers.
    """
    def __init__(self, *args, **kwargs):
        super(NumberedCanvas, self).__init__(*args, **kwargs)
        self._saved_page_states = []

    def showPage(self):
        self._saved_page_states.append(dict(self.__dict__))
        self._startPage()

    def save(self):
        num_pages = len(self._saved_page_states)
        for state in self._saved_page_states:
            self.__dict__.update(state)
            self.draw_page_decorations(num_pages)
            super(NumberedCanvas, self).showPage()
        super(NumberedCanvas, self).save()

    def draw_page_decorations(self, page_count):
        self.saveState()
        self.setFont("Helvetica", 8)
        self.setFillColor(colors.HexColor("#86868B"))

        # Skip headers and footers on the first page (Cover / Title)
        if self._pageNumber > 1:
            # Running Header
            self.drawString(36, 756, "MiniOS — Operating System Simulator | Live Website Demonstration Guide")
            self.drawRightString(576, 756, "Academic Defense & Viva Script")
            self.setStrokeColor(colors.HexColor("#E5E7EB"))
            self.setLineWidth(0.5)
            self.line(36, 750, 576, 750)

            # Running Footer
            self.line(36, 38, 576, 38)
            self.drawString(36, 28, "Confidential — Prepared for University Operating Systems Course Evaluation")
            page_text = f"Page {self._pageNumber} of {page_count}"
            self.drawRightString(576, 28, page_text)

        self.restoreState()


def create_demo_pdf():
    output_path = "/Users/aarushdubey/Downloads/MiniOS_Website_Demo_Guide.pdf"
    
    doc = SimpleDocTemplate(
        output_path,
        pagesize=letter,
        leftMargin=36,
        rightMargin=36,
        topMargin=42,
        bottomMargin=44
    )

    styles = getSampleStyleSheet()

    # Primary brand colors (Apple Minimal Light)
    PRIMARY = colors.HexColor("#0071E3")    # Signature Blue
    SECONDARY = colors.HexColor("#0066CC")  # Darker Accent Blue
    TEXT_DARK = colors.HexColor("#1D1D1F")  # Near-black
    TEXT_MUTED = colors.HexColor("#515154") # Slate gray
    BG_LIGHT = colors.HexColor("#F5F5F7")   # Neutral light gray
    BG_CARD = colors.HexColor("#FAFAFC")    # Subtle card surface
    BORDER = colors.HexColor("#E5E7EB")     # Crisp border
    GREEN = colors.HexColor("#027A48")      # Success green

    # Custom Typography Styles
    title_style = ParagraphStyle(
        'DocTitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=21,
        leading=25,
        textColor=TEXT_DARK,
        spaceAfter=4
    )

    subtitle_style = ParagraphStyle(
        'DocSubtitle',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=11,
        leading=14,
        textColor=PRIMARY,
        spaceAfter=10
    )

    h1_style = ParagraphStyle(
        'Heading1_Custom',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=13,
        leading=16,
        textColor=PRIMARY,
        spaceBefore=10,
        spaceAfter=4,
        keepWithNext=True
    )

    h2_style = ParagraphStyle(
        'Heading2_Custom',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=10.5,
        leading=14,
        textColor=TEXT_DARK,
        spaceBefore=7,
        spaceAfter=3,
        keepWithNext=True
    )

    body_style = ParagraphStyle(
        'Body_Custom',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8.5,
        leading=11.5,
        textColor=TEXT_DARK,
        spaceAfter=4
    )

    body_bold = ParagraphStyle(
        'Body_Bold',
        parent=body_style,
        fontName='Helvetica-Bold'
    )

    script_say_style = ParagraphStyle(
        'ScriptSay',
        parent=styles['Normal'],
        fontName='Helvetica-Oblique',
        fontSize=8.5,
        leading=11.5,
        textColor=colors.HexColor("#0F4C81")
    )

    action_style = ParagraphStyle(
        'ActionStep',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=8.5,
        leading=11.5,
        textColor=colors.HexColor("#027A48")
    )

    q_style = ParagraphStyle(
        'VivaQuestion',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=9,
        leading=12,
        textColor=colors.HexColor("#991B1B")
    )

    a_style = ParagraphStyle(
        'VivaAnswer',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8.5,
        leading=11.5,
        textColor=TEXT_DARK
    )

    story = []

    def make_callout(title_text, content_paragraph, bg_color=BG_CARD, border_color=PRIMARY, text_prefix=""):
        table_data = [[
            Paragraph(f"<b>{text_prefix} {title_text}</b>", body_bold),
        ], [
            content_paragraph
        ]]
        t = Table(table_data, colWidths=[540])
        t.setStyle(TableStyle([
            ('BACKGROUND', (0, 0), (-1, -1), bg_color),
            ('BOX', (0, 0), (-1, -1), 0.75, border_color),
            ('TOPPADDING', (0, 0), (-1, -1), 4),
            ('BOTTOMPADDING', (0, 0), (-1, -1), 4),
            ('LEFTPADDING', (0, 0), (-1, -1), 7),
            ('RIGHTPADDING', (0, 0), (-1, -1), 7),
            ('INNERGRID', (0, 0), (-1, -1), 0.5, colors.transparent),
        ]))
        return t

    # ==========================================
    # COVER / HEADER SECTION
    # ==========================================
    story.append(Paragraph("MiniOS Web Laboratory — Professor Demonstration & Viva Script", title_style))
    story.append(Paragraph("A Step-by-Step Interactive Presentation Guide for GitHub Pages Deployment & Academic Review", subtitle_style))
    story.append(HRFlowable(width="100%", thickness=1.2, color=PRIMARY, spaceBefore=0, spaceAfter=8))

    # Meta Information Box
    meta_data = [
        [
            Paragraph("<b>Project:</b> MiniOS Simulator", body_style),
            Paragraph("<b>Target Audience:</b> Course Professor & Viva Panel", body_style),
            Paragraph("<b>Live URL:</b> <font color='#0071E3'><u>https://aarush-lab9.github.io/MiniOS/</u></font>", body_style)
        ],
        [
            Paragraph("<b>Design Theme:</b> Apple Minimal Light", body_style),
            Paragraph("<b>Simulation Engine:</b> Pure Client-Side JS (Zero Backend)", body_style),
            Paragraph("<b>Core Mirror:</b> POSIX C Kernel (13 C/H files)", body_style)
        ]
    ]
    meta_table = Table(meta_data, colWidths=[175, 190, 175])
    meta_table.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, -1), BG_LIGHT),
        ('BOX', (0, 0), (-1, -1), 0.5, BORDER),
        ('INNERGRID', (0, 0), (-1, -1), 0.5, BORDER),
        ('TOPPADDING', (0, 0), (-1, -1), 3),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 3),
        ('LEFTPADDING', (0, 0), (-1, -1), 5),
        ('RIGHTPADDING', (0, 0), (-1, -1), 5),
    ]))
    story.append(meta_table)
    story.append(Spacer(1, 6))

    # ==========================================
    # SECTION 1: EXECUTIVE BRIEFING & 30-SEC OPENING
    # ==========================================
    story.append(Paragraph("1. Executive Briefing & Opening Pitch (First 30 Seconds)", h1_style))
    story.append(Paragraph(
        "Start your presentation with confidence. Before clicking any button, give the professor a 30-second conceptual summary explaining why this simulator was built, how it works, and what makes it technically authentic.",
        body_style
    ))

    speech_p1 = Paragraph(
        '"Good morning Professor. Today I am excited to demonstrate <b>MiniOS</b> — an interactive operating system laboratory. '
        'In traditional OS coursework, concepts like priority preemption, multilevel feedback queues, memory coalescing, and paging are studied as abstract mathematical formulas. '
        'I built MiniOS to make these concepts tangible through a <b>dual-architecture implementation</b>: a high-performance C kernel that executes natively in the terminal, '
        'and a client-side web laboratory hosted on GitHub Pages that mathematically mirrors the exact same logic with zero external server dependencies. '
        'The web interface follows an Apple Minimal Light design with an embedded real-time monospace terminal. '
        'Over the next few minutes, I will walk you through live simulations of all six core operating system subsystems."',
        script_say_style
    )
    story.append(make_callout("What to Say to the Professor (The 30-Second Hook):", speech_p1, bg_color=BG_CARD, border_color=PRIMARY, text_prefix="🗣️"))
    story.append(Spacer(1, 6))

    # Key Architectural Highlights
    arch_data = [
        [
            Paragraph("<b>Subsystem</b>", body_bold),
            Paragraph("<b>Underlying OS Concept</b>", body_bold),
            Paragraph("<b>What the Simulator Visualizes</b>", body_bold)
        ],
        [
            Paragraph("<b>1. CPU Scheduling</b>", body_style),
            Paragraph("Process Control Block (PCB), 5-state lifecycle, preemption", body_style),
            Paragraph("Horizontal Gantt chart timeline, Turnaround & Waiting time metrics", body_style)
        ],
        [
            Paragraph("<b>2. Memory Management</b>", body_style),
            Paragraph("Contiguous variable partitions, First/Best/Worst Fit", body_style),
            Paragraph("1024 MB arena layout, external fragmentation, O(1) hole coalescing", body_style)
        ],
        [
            Paragraph("<b>3. Concurrency & Deadlocks</b>", body_style),
            Paragraph("Counting/Binary Semaphores, Dijkstra's Banker's Algorithm", body_style),
            Paragraph("5-slot ring buffer state, Allocation/Need matrices, Safe sequence", body_style)
        ],
        [
            Paragraph("<b>4. Virtual Memory Paging</b>", body_style),
            Paragraph("Demand paging, page faults, temporal locality, Bélády's anomaly", body_style),
            Paragraph("Step-by-step frame contents, Hit vs. Fault detection, Hit Ratio %", body_style)
        ],
        [
            Paragraph("<b>5. Secondary Storage Disk</b>", body_style),
            Paragraph("Mechanical head seek time, 200-cylinder geometry (0–199)", body_style),
            Paragraph("Actuator trajectories (FCFS, SSTF, SCAN, C-SCAN), total seek travel", body_style)
        ]
    ]
    arch_table = Table(arch_data, colWidths=[120, 200, 220])
    arch_table.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, 0), BG_LIGHT),
        ('BOX', (0, 0), (-1, -1), 0.5, BORDER),
        ('INNERGRID', (0, 0), (-1, -1), 0.5, BORDER),
        ('TOPPADDING', (0, 0), (-1, -1), 3),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 3),
        ('LEFTPADDING', (0, 0), (-1, -1), 5),
        ('RIGHTPADDING', (0, 0), (-1, -1), 5),
    ]))
    story.append(arch_table)
    story.append(Spacer(1, 8))

    # ==========================================
    # SECTION 2: SHOWSTOPPER — 1-CLICK GUIDED DEMO
    # ==========================================
    story.append(Paragraph("2. The Showstopper: The Guided 24-Step Demonstration", h1_style))
    story.append(Paragraph(
        "The fastest way to impress your professor before manually clicking inputs is to trigger the built-in <b>automated demonstration engine</b>. "
        "This proves that the entire suite functions coherently as an integrated operating system.",
        body_style
    ))

    act_demo = Paragraph(
        'Click the blue <b>"Run Demonstration"</b> button in the top navigation bar (or the <b>"Start Demonstration"</b> button on the Overview card). '
        'Watch the simulated terminal stream the 24 sequential steps with 1-second automated pacing.',
        action_style
    )
    story.append(make_callout("Action to Perform on Screen:", act_demo, bg_color=BG_CARD, border_color=GREEN, text_prefix="🎯"))
    story.append(Spacer(1, 4))

    speech_demo = Paragraph(
        '"Professor, before we inspect individual modules, I would like to demonstrate our automated 24-step curriculum. '
        'When I click <b>Run Demonstration</b>, MiniOS enters an automated execution mode. '
        'Notice how the terminal automatically initializes the Process Table, creates workloads P1 through P5, runs side-by-side scheduling benchmarks, '
        'allocates memory across First-Fit and Best-Fit, releases blocks to prove O(1) coalescing, executes Dijkstra\'s semaphores for Producer-Consumer, '
        'calculates Banker\'s safety matrices, simulates demand paging, and graphs disk arm seeks. '
        'The button state transitions to a live step counter, allowing anyone evaluating this simulator to verify the full OS pipeline hands-free."',
        script_say_style
    )
    story.append(make_callout("What to Say During the Automated Run:", speech_demo, bg_color=BG_CARD, border_color=PRIMARY, text_prefix="🗣️"))
    story.append(Spacer(1, 8))

    # ==========================================
    # SECTION 3: TAB-BY-TAB WALKTHROUGH
    # ==========================================
    story.append(Paragraph("3. Tab-by-Tab Deep Dive & Interactive Demonstration", h1_style))
    story.append(Paragraph(
        "After the overview, systematically walk through each tab using the Apple segmented control bar. Here is the exact script and test cases for each module.",
        body_style
    ))

    # MODULE 1: CPU SCHEDULING
    story.append(Paragraph("Module A: CPU Scheduling & Process Management", h2_style))
    story.append(Paragraph(
        "<b>Goal:</b> Prove how Process Control Blocks (PCBs) transition through lifecycle states, demonstrate CPU preemption, and compare algorithm efficiencies using ASCII Gantt charts.",
        body_style
    ))

    act_cpu = Paragraph(
        '1. Click the <b>"CPU Scheduling"</b> segment tab.<br/>'
        '2. Click <b>"Load Workload (P1–P5)"</b> — automatically populates 5 realistic processes with staggered arrival times and varying priorities.<br/>'
        '3. Click <b>"Inspect Process Table"</b> to show the current PCB states in the terminal.<br/>'
        '4. Click <b>"Compare All Scheduling Policies Side-by-Side"</b>.',
        action_style
    )
    story.append(make_callout("Live Demo Actions:", act_cpu, bg_color=BG_CARD, border_color=GREEN, text_prefix="🎯"))
    story.append(Spacer(1, 4))

    speech_cpu = Paragraph(
        '"Here in the CPU Scheduling module, we maintain an authentic Process Control Block table. '
        'Each process stores its PID, arrival time, burst duration, static priority, and current state. '
        'When I click <b>Compare All</b>, MiniOS runs the exact same workload through five distinct dispatching algorithms under identical initial conditions: '
        '<br/>• <b>FCFS:</b> Simple FIFO queue, but suffers from the <i>Convoy Effect</i> if an early job has a long burst. '
        '<br/>• <b>SJF:</b> Mathematically provable minimum average waiting time, but susceptible to starvation of long jobs. '
        '<br/>• <b>Round Robin (q=2):</b> Provides fair interactive response time via preemptive time-slicing. '
        '<br/>• <b>Priority:</b> Dispatches highest priority first (lower numerical integer = higher urgency). '
        '<br/>• <b>MLFQ:</b> A 3-tier feedback queue (Q0: RR q=1, Q1: RR q=2, Q2: FCFS). Notice how I/O bound jobs finish quickly in Q0, while CPU-intensive jobs are dynamically demoted to lower priority queues without starvation. '
        'The terminal renders an ASCII Gantt chart timeline and computes Turnaround Time (Completion - Arrival) and Waiting Time (Turnaround - Burst) for each algorithm."',
        script_say_style
    )
    story.append(make_callout("What to Explain to the Professor:", speech_cpu, bg_color=BG_CARD, border_color=PRIMARY, text_prefix="🗣️"))
    story.append(Spacer(1, 8))

    # MODULE 2: MEMORY MANAGEMENT
    story.append(Paragraph("Module B: 1024 MB Dynamic Memory Arena", h2_style))
    story.append(Paragraph(
        "<b>Goal:</b> Demonstrate dynamic variable-partition allocation, compare placement heuristics (First-Fit, Best-Fit, Worst-Fit), and explain O(1) bidirectional hole coalescing.",
        body_style
    ))

    act_mem = Paragraph(
        '1. Click the <b>"Memory Arena"</b> segment tab.<br/>'
        '2. Set Name: <b>P1</b>, Size: <b>200</b> MB, Policy: <b>First Fit</b>. Click <b>"Allocate Block"</b>.<br/>'
        '3. Set Name: <b>P2</b>, Size: <b>300</b> MB, Policy: <b>Best Fit</b>. Click <b>"Allocate Block"</b>.<br/>'
        '4. Set Name: <b>P3</b>, Size: <b>150</b> MB, Policy: <b>Worst Fit</b>. Click <b>"Allocate Block"</b>.<br/>'
        '5. Click <b>"Inspect Memory Arena"</b> to show the partition map.<br/>'
        '6. Set Process Tag: <b>P2</b>, click <b>"Free Block"</b>.<br/>'
        '7. Click <b>"Inspect Memory Arena"</b> again to show how P2 became a free hole, and free P1 and P3 to demonstrate automatic coalescing.',
        action_style
    )
    story.append(make_callout("Live Demo Actions:", act_mem, bg_color=BG_CARD, border_color=GREEN, text_prefix="🎯"))
    story.append(Spacer(1, 4))

    speech_mem = Paragraph(
        '"In this module, MiniOS manages a 1024 MB contiguous physical RAM pool modeled as a doubly-linked list of <code>MemBlock</code> structures. '
        'We support three classic placement policies: '
        '<br/>• <b>First-Fit:</b> Scans from lowest physical offset and allocates the first hole of sufficient size. Fastest heuristic with minimal search overhead. '
        '<br/>• <b>Best-Fit:</b> Scans the entire list to find the smallest hole that satisfies the request, minimizing wasted residual space at the cost of leaving tiny unallocatable slivers. '
        '<br/>• <b>Worst-Fit:</b> Allocates from the largest available hole so that the remaining split portion remains large enough to service future jobs. '
        'Most importantly, look at what happens when I deallocate P2 and adjacent blocks: MiniOS inspects <code>blk->prev</code> and <code>blk->next</code>. '
        'If either neighbor is an unallocated hole, it merges them in <b>O(1) time</b>, eliminating external fragmentation without expensive full-arena compaction."',
        script_say_style
    )
    story.append(make_callout("What to Explain to the Professor:", speech_mem, bg_color=BG_CARD, border_color=PRIMARY, text_prefix="🗣️"))
    story.append(Spacer(1, 8))

    # MODULE 3: CONCURRENCY & DEADLOCKS
    story.append(Paragraph("Module C: Process Synchronization & Concurrency Control", h2_style))
    story.append(Paragraph(
        "<b>Goal:</b> Prove mutual exclusion and synchronization using counting and binary semaphores in a bounded buffer, and demonstrate multi-resource deadlock avoidance via Banker's algorithm.",
        body_style
    ))

    act_sync = Paragraph(
        '1. Click the <b>"Synchronization"</b> segment tab.<br/>'
        '2. In the Producer-Consumer card, click <b>"Run Full Simulation"</b>.<br/>'
        '3. Show how the buffer state updates across all 10 interleaved steps.<br/>'
        '4. In the Banker\'s Algorithm card, click <b>"Evaluate Banker\'s Safety"</b>.',
        action_style
    )
    story.append(make_callout("Live Demo Actions:", act_sync, bg_color=BG_CARD, border_color=GREEN, text_prefix="🎯"))
    story.append(Spacer(1, 4))

    speech_sync = Paragraph(
        '"Here we address two foundational concurrency challenges in operating systems: '
        '<br/><b>1. Producer-Consumer Problem:</b> We simulate a 5-slot circular ring buffer guarded by Dijkstra\'s semaphores. '
        'We use two counting semaphores — <code>empty=5</code> (available slots) and <code>full=0</code> (data items) — and one binary semaphore <code>mutex=1</code> for critical section protection. '
        'Notice how the step-by-step trace shows atomic <code>wait()</code> and <code>signal()</code> operations. If the buffer is full, the producer is safely suspended; if empty, the consumer blocks, preventing race conditions and buffer overruns. '
        '<br/><b>2. Banker\'s Algorithm:</b> This models deadlock <i>avoidance</i> across 5 processes (P0–P4) and 3 resource classes (A, B, C). '
        'MiniOS computes the <code>Need Matrix = Max - Allocation</code> and initializes <code>Work = Available</code>. '
        'The safety engine iteratively searches for an uncompleted process whose <code>Need &le; Work</code>. '
        'When found, it simulates execution and reclaims its held resources (<code>Work += Allocation</code>). '
        'As you can see on the terminal, the algorithm discovers the guaranteed safe sequence <b>&lang; P1 &rarr; P3 &rarr; P4 &rarr; P0 &rarr; P2 &rang;</b>, proving that the system is free from circular wait."',
        script_say_style
    )
    story.append(make_callout("What to Explain to the Professor:", speech_sync, bg_color=BG_CARD, border_color=PRIMARY, text_prefix="🗣️"))
    story.append(Spacer(1, 8))

    # MODULE 4: VIRTUAL MEMORY PAGING
    story.append(Paragraph("Module D: Virtual Memory Page Replacement", h2_style))
    story.append(Paragraph(
        "<b>Goal:</b> Demonstrate demand paging, illustrate physical frame contents after each reference, and explain the difference between FIFO, LRU, and Bélády's Optimal algorithms.",
        body_style
    ))

    act_page = Paragraph(
        '1. Click the <b>"Virtual Memory"</b> segment tab.<br/>'
        '2. Keep Frames = <b>3</b> and Reference String = <b>7,0,1,2,0,3,0,4,2,3,0,3</b>.<br/>'
        '3. Click <b>"Compare Page Policies"</b>.<br/>'
        '4. Click the preset pill <b>"Bélády’s Anomaly String"</b> (<code>1,2,3,4,1,2,5,1,2,3,4,5</code>) and discuss why increasing frames can increase faults under FIFO.',
        action_style
    )
    story.append(make_callout("Live Demo Actions:", act_page, bg_color=BG_CARD, border_color=GREEN, text_prefix="🎯"))
    story.append(Spacer(1, 4))

    speech_page = Paragraph(
        '"In this module, MiniOS models virtual memory demand paging. '
        'When a logical page address is referenced, the MMU checks if it is resident in physical frames. If present, we register a <b>Page Hit</b> with zero I/O cost; if absent, a <b>Page Fault</b> triggers victim eviction. '
        '<br/>• <b>FIFO:</b> Evicts the oldest loaded page using a circular pointer queue. Simple to implement in hardware, but ignores frequency and recency. '
        '<br/>• <b>LRU:</b> Evicts the page unused for the longest time, exploiting <i>temporal locality</i>. We track step timestamps for each frame. Notice how LRU significantly reduces page faults compared to FIFO. '
        '<br/>• <b>Bélády\'s Optimal:</b> Looks into future references and evicts the page that will not be used for the longest future duration. While unrealizable in practice because the kernel cannot predict the future, it provides the mathematical theoretical minimum fault ceiling for benchmarking. '
        'We also provide a preset for <b>Bélády\'s Anomaly</b>, demonstrating how FIFO can counter-intuitively experience MORE page faults when physical memory is increased from 3 to 4 frames."',
        script_say_style
    )
    story.append(make_callout("What to Explain to the Professor:", speech_page, bg_color=BG_CARD, border_color=PRIMARY, text_prefix="🗣️"))
    story.append(Spacer(1, 8))

    # MODULE 5: DISK SCHEDULING
    story.append(Paragraph("Module E: Secondary Storage Disk Arm Scheduling", h2_style))
    story.append(Paragraph(
        "<b>Goal:</b> Visualize mechanical read/write head trajectories over 200 cylinders and quantify seek reduction between FCFS, SSTF, SCAN, and C-SCAN.",
        body_style
    ))

    act_disk = Paragraph(
        '1. Click the <b>"Storage"</b> segment tab.<br/>'
        '2. Verify Initial Head = <b>53</b> and Requests = <b>98,183,37,122,14,124,65,67</b>.<br/>'
        '3. Click <b>"Compare Seek Trajectories"</b>.',
        action_style
    )
    story.append(make_callout("Live Demo Actions:", act_disk, bg_color=BG_CARD, border_color=GREEN, text_prefix="🎯"))
    story.append(Spacer(1, 4))

    speech_disk = Paragraph(
        '"Finally, the Storage module simulates hard disk cylinder geometry spanning cylinders 0 through 199. '
        'Mechanical seek time is the primary performance bottleneck in secondary storage. MiniOS calculates the absolute head travel distance: <code>&Sigma; |target - current|</code>. '
        '<br/>• <b>FCFS:</b> Services requests in order of arrival. For our test workload, the head thrashes wildly back and forth across 640 cylinders. '
        '<br/>• <b>SSTF (Shortest Seek Time First):</b> Greedily selects the closest cylinder to the current head position. Notice that total seek distance drops drastically from 640 to <b>236 cylinders</b> — an impressive <b>63% reduction</b> in mechanical latency. However, SSTF risks cylinder starvation if new requests cluster near the head. '
        '<br/>• <b>SCAN (Elevator Algorithm):</b> Sweeps continuously in one direction towards cylinder 199, servicing requests along the path, then reverses direction. This guarantees bounded waiting time. '
        '<br/>• <b>C-SCAN (Circular SCAN):</b> Sweeps in one direction to the disk boundary (199), then immediately jumps back to cylinder 0 without servicing requests on the return trip. This provides a more uniform waiting time distribution for requests at both extremes of the platter."',
        script_say_style
    )
    story.append(make_callout("What to Explain to the Professor:", speech_disk, bg_color=BG_CARD, border_color=PRIMARY, text_prefix="🗣️"))
    story.append(Spacer(1, 8))

    # ==========================================
    # SECTION 4: CLI DEMO & COMMAND BAR
    # ==========================================
    story.append(Paragraph("4. Interactive Monospace Terminal Demonstration", h1_style))
    story.append(Paragraph(
        "To prove that the GitHub Pages site isn't just a static form, show your professor the <b>interactive command bar</b> at the bottom of the macOS terminal window. You can type raw commands just like in a native shell.",
        body_style
    ))

    act_cli = Paragraph(
        '1. Scroll to the bottom of the right-hand terminal window.<br/>'
        '2. Click into the <code>minios&gt;</code> input bar.<br/>'
        '3. Type: <code>schedule compare</code> and press <b>Enter</b> (or click Send).<br/>'
        '4. Type: <code>bankers</code> and press <b>Enter</b>.<br/>'
        '5. Type: <code>mem_status</code> and press <b>Enter</b>.<br/>'
        '6. Click the <b>"Copy"</b> button on the macOS window titlebar to demonstrate clipboard export.',
        action_style
    )
    story.append(make_callout("CLI Demo Actions:", act_cli, bg_color=BG_CARD, border_color=GREEN, text_prefix="🎯"))
    story.append(Spacer(1, 4))

    speech_cli = Paragraph(
        '"Notice, Professor, that on the right side of the screen, we have a fully interactive macOS-styled terminal. '
        'Users are not limited to clicking GUI buttons. I can type standard MiniOS commands into the <code>minios&gt;</code> prompt bar. '
        'For instance, typing <code>schedule compare</code> or <code>bankers</code> executes the command line parser, just like running <code>./minios</code> in a Linux or macOS shell. '
        'Students and evaluators can also click the <b>Copy</b> button on the window titlebar to export simulation traces directly into laboratory reports."',
        script_say_style
    )
    story.append(make_callout("What to Say for the CLI Demonstration:", speech_cli, bg_color=BG_CARD, border_color=PRIMARY, text_prefix="🗣️"))
    story.append(Spacer(1, 8))

    # ==========================================
    # SECTION 5: ANTICIPATED VIVA QUESTIONS
    # ==========================================
    story.append(Paragraph("5. Top 10 Anticipated Professor Viva Questions & Answers", h1_style))
    story.append(Paragraph(
        "Professors evaluate OS projects by probing the theoretical foundations and design decisions. Below are the 10 most common viva questions with exact, academically rigorous answers.",
        body_style
    ))

    viva_qa = [
        (
            "Q1: Why is Shortest Job First (SJF) mathematically optimal for waiting time, and why can't general-purpose OS kernels implement pure SJF?",
            "SJF is provably optimal because scheduling the job with the shortest CPU burst first minimizes the sum of waiting times for all remaining jobs in the ready queue. However, general-purpose kernels cannot implement pure SJF because the exact duration of a future CPU burst cannot be known in advance before the process executes. Instead, real systems approximate SJF using exponential moving average burst prediction: &tau;_{n+1} = &alpha; t_n + (1 - &alpha;) &tau;_n, or use Multilevel Feedback Queues (MLFQ) which dynamically discover burst lengths based on whether jobs exhaust their time quantum."
        ),
        (
            "Q2: What is the 'Convoy Effect' in FCFS scheduling, and how does Round Robin resolve it?",
            "The convoy effect occurs in non-preemptive FCFS when a single CPU-bound process with a huge burst time acquires the processor ahead of multiple short I/O-bound jobs. All shorter jobs are forced to queue behind the long job, causing CPU utilization and I/O device utilization to plummet while average waiting time spikes. Round Robin resolves this by introducing a time quantum: when the quantum expires, the long process is preempted and returned to the rear of the ready queue, allowing short interactive jobs to interleave and complete quickly."
        ),
        (
            "Q3: How does your 3-Tier Multilevel Feedback Queue (MLFQ) prevent CPU-bound jobs from starving I/O-bound jobs?",
            "Our MLFQ simulator implements three distinct queue levels: Queue 0 has highest priority with RR quantum = 1 ms, Queue 1 has medium priority with RR quantum = 2 ms, and Queue 2 has lowest priority with FCFS dispatching. Newly arrived jobs always enter Queue 0. Short interactive jobs finish within 1 ms and terminate immediately. Long compute-bound jobs exhaust their time quantum and are demoted down to Queue 1 and eventually Queue 2. Higher priority queues always preempt lower queues, ensuring interactive responsiveness while background batch jobs still make progress."
        ),
        (
            "Q4: What is the exact difference between Internal and External Fragmentation in memory management?",
            "Internal fragmentation occurs in fixed-partition or paging systems when physical memory allocated to a process is larger than the requested size; the unutilized memory is wasted *inside* the allocated boundary. External fragmentation occurs in variable-partition dynamic contiguous allocation when sufficient total free memory exists across the arena to satisfy a request, but the memory is broken into non-contiguous holes scattered across physical offsets, preventing any single contiguous allocation. Our simulator visualizes external fragmentation in the memory map."
        ),
        (
            "Q5: How does your memory deallocation achieve O(1) time complexity for hole coalescing?",
            "MiniOS models the 1024 MB physical memory pool as a doubly-linked list where each partition node maintains explicit `prev` and `next` pointers to its physical neighbors. When a block is freed, the deallocator does not search or re-scan the entire 1024 MB pool. It simply checks `blk->prev->allocated == 0` and `blk->next->allocated == 0`. If a neighbor is free, it merges the nodes by adjusting the linked list pointers and adding their sizes in constant time O(1)."
        ),
        (
            "Q6: Why is Banker's Algorithm classified as Deadlock 'Avoidance' rather than 'Prevention' or 'Detection'?",
            "Deadlock *Prevention* works by strictly constraining resource request protocols to mathematically eliminate at least one of Coffman's four conditions (e.g., denying hold-and-wait or enforcing total resource ordering). Deadlock *Detection* allows deadlocks to occur freely, running periodic graph-cycle algorithms to detect and abort deadlocked processes. In contrast, Banker's Algorithm is *Avoidance*: processes declare maximum resource claims upfront, and for every dynamic allocation request, the kernel evaluates whether granting the request keeps the system in a 'Safe State' (where a safe sequence exists to satisfy all maximum claims)."
        ),
        (
            "Q7: What is the difference between Counting Semaphores and Binary Semaphores (Mutexes)?",
            "A binary semaphore (mutex) has an integer value restricted to 0 and 1, used exclusively to enforce mutual exclusion inside a critical section so that only one thread executes at a time. A counting semaphore has an integer value span over an unrestricted positive range [0, N], used to coordinate resource availability across a fixed pool of instances. In our Producer-Consumer simulation, `mutex=1` protects buffer read/write access, while `empty=5` and `full=0` track available and occupied slots."
        ),
        (
            "Q8: What is Bélády's Anomaly, and why does the Least Recently Used (LRU) algorithm never suffer from it?",
            "Bélády's Anomaly is the counter-intuitive phenomenon where increasing the number of physical frame slots allocated to a process results in an INCREASE in the total number of page faults under certain reference strings. First-In, First-Out (FIFO) suffers from this anomaly because its replacement choice depends on arrival order rather than locality. LRU never suffers from Bélády's Anomaly because it belongs to the class of *Stack Algorithms*: the set of pages resident in an n-frame allocation is always a strict subset of the pages resident in an (n+1)-frame allocation."
        ),
        (
            "Q9: Why does C-SCAN provide more uniform waiting times across disk cylinders than standard SCAN?",
            "Under standard SCAN (Elevator algorithm), the disk arm sweeps back and forth across the platter. When the head reverses direction at cylinder 199, cylinders near the extreme boundary were serviced very recently and have few pending requests, whereas cylinders at the opposite end of the disk (near cylinder 0) have been waiting the longest. Circular SCAN (C-SCAN) fixes this asymmetry by treating the cylinders as a circular list: it sweeps in one direction only, then immediately jumps back to cylinder 0 without servicing requests on the return. This ensures every cylinder experiences equal inter-arrival service intervals."
        ),
        (
            "Q10: Why did you disable standard I/O stream buffering (`_IONBF`) in your C kernel simulator?",
            "In standard C, `stdout` is line-buffered when attached to an interactive terminal, but block-buffered (typically 4096 bytes) when connected to a pipe, redirect, or automated harness. Disabling stream buffering using `setvbuf(stdout, NULL, _IONBF, 0)` forces immediate, synchronous unbuffered flushes after every `printf()`. This guarantees that Gantt charts, memory maps, and step traces stream in real-time without latency or missing final lines."
        )
    ]

    for q, a in viva_qa:
        q_p = Paragraph(f"<b>{q}</b>", q_style)
        a_p = Paragraph(f"<b>Model Answer:</b> {a}", a_style)
        qa_data = [[q_p], [a_p]]
        qa_t = Table(qa_data, colWidths=[540])
        qa_t.setStyle(TableStyle([
            ('BACKGROUND', (0, 0), (-1, -1), BG_LIGHT),
            ('BOX', (0, 0), (-1, -1), 0.5, BORDER),
            ('TOPPADDING', (0, 0), (-1, -1), 3),
            ('BOTTOMPADDING', (0, 0), (-1, -1), 3),
            ('LEFTPADDING', (0, 0), (-1, -1), 6),
            ('RIGHTPADDING', (0, 0), (-1, -1), 6),
        ]))
        story.append(qa_t)
        story.append(Spacer(1, 3))

    # ==========================================
    # SECTION 6: CONCLUDING CLOSING STATEMENT
    # ==========================================
    story.append(Spacer(1, 5))
    story.append(Paragraph("6. Professional Concluding Statement (The Wrap-Up)", h1_style))
    speech_end = Paragraph(
        '"To conclude, Professor: MiniOS bridges textbook operating system theory with verifiable software engineering. '
        'Every algorithm — from MLFQ feedback demotion and O(1) hole coalescing to Banker\'s matrix verification and demand paging — '
        'has been modeled with mathematical fidelity in both our POSIX C core and our client-side web laboratory. '
        'The deployment on GitHub Pages ensures that students can access and experiment with these concepts on any device without complex environment setup. '
        'Thank you for your time, and I welcome any technical questions or live test cases you would like to test on the system."',
        script_say_style
    )
    story.append(make_callout("Closing Remarks to Deliver:", speech_end, bg_color=BG_CARD, border_color=PRIMARY, text_prefix="🗣️"))

    # Build Document
    doc.build(story, canvasmaker=NumberedCanvas)
    print(f"Successfully generated demo guide PDF at: {output_path}")

if __name__ == "__main__":
    create_demo_pdf()
