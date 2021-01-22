import unittest
import uuid

import gtirb

import gtirb_functions


class FunctionTest(unittest.TestCase):
    def _generate_subgraph(self, interval, *edges):
        blocks = {}
        for src, dst, *label_args in edges:
            if src not in blocks:
                blocks[src] = gtirb.CodeBlock(offset=src, size=1)
                blocks[src].byte_interval = interval
            b1 = blocks[src]
            if dst not in blocks:
                blocks[dst] = gtirb.CodeBlock(offset=dst, size=1)
                blocks[dst].byte_interval = interval
            b2 = blocks[dst]

            label = gtirb.Edge.Label(*label_args)
            interval.ir.cfg.add(gtirb.Edge(b1, b2, label))

        return sorted(blocks.values(), key=lambda b: b.offset)

    def test_build_functions(self):
        ir = gtirb.IR()
        module = gtirb.Module(name="test")
        module.ir = ir
        section = gtirb.Section(name=".text")
        section.module = module

        interval = gtirb.ByteInterval(address=0x1000, size=0)
        interval.section = section

        f1 = uuid.uuid4()
        f1_blocks = self._generate_subgraph(
            interval,
            (0, 1, gtirb.Edge.Type.Fallthrough),
            (0, 2, gtirb.Edge.Type.Branch, True),
            (1, 2, gtirb.Edge.Type.Fallthrough),
            (2, 3, gtirb.Edge.Type.Return),
        )
        f1_name = gtirb.Symbol(name="f1", payload=f1_blocks[0])
        f1_name.module = module

        f2 = uuid.uuid4()
        f2_blocks = self._generate_subgraph(
            interval, (10, 11, gtirb.Edge.Type.Return),
        )
        f2_name = gtirb.Symbol(name="f2", payload=f2_blocks[0])
        f2_name.module = module

        module.aux_data["functionBlocks"] = gtirb.AuxData(
            type_name="mapping<UUID,set<UUID>>",
            data={f1: set(f1_blocks[:-1]), f2: set(f2_blocks[:-1])},
        )

        module.aux_data["functionEntries"] = gtirb.AuxData(
            type_name="mapping<UUID,set<UUID>>",
            data={f1: {f1_blocks[0]}, f2: {f2_blocks[0]}},
        )

        matches = 0
        for fun in gtirb_functions.Function.build_functions(module):
            if fun.get_name() == "f1":
                self.assertEqual(fun.uuid, f1)
                self.assertEqual(fun.get_entry_blocks(), {f1_blocks[0]})
                self.assertEqual(fun.get_exit_blocks(), {f1_blocks[-2]})
                self.assertEqual(fun.get_all_blocks(), set(f1_blocks[:-1]))
                matches += 1
            elif fun.get_name() == "f2":
                self.assertEqual(fun.uuid, f2)
                self.assertEqual(fun.get_entry_blocks(), {f2_blocks[0]})
                self.assertEqual(fun.get_exit_blocks(), {f2_blocks[-2]})
                self.assertEqual(fun.get_all_blocks(), set(f2_blocks[:-1]))
                matches += 1
        self.assertEqual(matches, 2)
